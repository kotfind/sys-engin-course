#include "fuse_fs.hpp"
#include "fuse_dir.hpp"
#include "fuse_file.hpp"
#include "helpers.hpp"
#include "log.hpp"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>
#include <sys/types.h>
#include <thread>
#include <utility>
#include <variant>

#define FUSE_USE_VERSION 32
#include <fuse.h>
#include <fuse_opt.h>

#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
struct fuse_operations FuseFs::fuse_operations = {
    .getattr = FuseFs::fuse_getattr,
    .truncate = FuseFs::fuse_truncate,
    .open = FuseFs::fuse_open,
    .read = FuseFs::fuse_read,
    .write = FuseFs::fuse_write,
    .flush = FuseFs::fuse_flush,
    .readdir = FuseFs::fuse_readdir,
    .init = FuseFs::fuse_init,
};

FuseFs::FuseFs(std::unique_ptr<FuseDir> root)
    : fuse(nullptr), root(std::move(root)) {
}

FuseFs::~FuseFs() {
    if (this->fuse_args.argc != 0) {
        fuse_opt_free_args(&this->fuse_args);
    }

    if (this->fuse != nullptr) {
        fuse_unmount(this->fuse);
        fuse_destroy(this->fuse);
    }
}

FuseFs* FuseFs::mount(
    const std::filesystem::path& mount_path_, std::unique_ptr<FuseDir> root
) {
    auto mount_path = std::filesystem::absolute(mount_path_).lexically_normal();
    info("Trying to mount at `{}`", mount_path.string());

    auto fs = std::unique_ptr<FuseFs>(new FuseFs(std::move(root)));

    {
        static char dummy[] = "dummy\0";
        static char* dummy_argv[] = {dummy, NULL};
        fs->fuse_args = FUSE_ARGS_INIT(1, dummy_argv);
    }

    fs->fuse = fuse_new(
        &fs->fuse_args, &fuse_operations, sizeof(fuse_operations), fs.get()
    );
    if (fs->fuse == nullptr) {
        error("Failed to init FUSE");
        return nullptr;
    }

    if (fuse_mount(fs->fuse, mount_path.c_str()) != 0) {
        error("Failed to mount FUSE");
        return nullptr;
    }

    {
        auto* fuse = fs->fuse;
        std::thread([fuse] { fuse_loop(fuse); }).detach();
    }

    success("Mounted at `{}`", mount_path.string());
    return fs.release();
}

bool FuseFs::invalidate_path(std::string_view path) {
    auto status = fuse_invalidate_path(this->fuse, path.data());
    return status == 0 || status == -ENOENT;
}

bool FuseFs::set_file_read_data(
    std::string_view path, std::span<const std::byte> data
) {
    std::unique_lock lock(this->mutex);

    auto* file = this->root->get_file(path);
    if (file == nullptr) {
        return false;
    }

    file->set_read_data(data, this, path);

    return true;
}

std::pair<FuseFs*, std::unique_lock<std::mutex>> FuseFs::get_fs_locked() {
    auto* ctx = fuse_get_context();
    assert(ctx != nullptr);

    auto* fs = (FuseFs*)ctx->private_data;
    assert(fs != nullptr);

    std::unique_lock lock{fs->mutex};

    return {fs, std::move(lock)};
}

void* FuseFs::fuse_init(fuse_conn_info* con, fuse_config* cfg) {
    (void)con;

    constexpr double SOME_BIG_NUMBER_SECS = 1'000'000;

    // We invalidate cache manually on each file modification,
    // so timeout mechanism can be "disabled".

    cfg->entry_timeout = SOME_BIG_NUMBER_SECS;
    cfg->entry_timeout = SOME_BIG_NUMBER_SECS;
    cfg->negative_timeout = 0;

    return fuse_get_context()->private_data;
}

int FuseFs::fuse_open(const char* path, fuse_file_info* file_info) {
    trace("FUSE request: open {}", path);

    if (file_info->flags & O_TRUNC) {
        fuse_truncate(path, 0, file_info);
    }

    return 0;
}

int FuseFs::fuse_flush(const char* path, struct fuse_file_info* file_info) {
    (void)file_info;

    trace("FUSE request: flush {}", path);

    auto [fs, lock] = get_fs_locked();

    auto* file = fs->root->get_file(path);
    if (file == nullptr) {
        return 0;
    }

    file->after_close(fs, path);
    fs->invalidate_path(path);

    return 0;
}

int FuseFs::fuse_getattr(
    const char* path, struct stat* stat, fuse_file_info* file_info
) {
    (void)file_info;

    trace("FUSE request: getattr {}", path);

    auto [fs, lock] = get_fs_locked();

    auto entry = fs->root->get_entry(path);
    return std::visit(
        overloads{
            [stat](FuseFile* file) {
                stat->st_mode = S_IFREG | 0444;
                stat->st_nlink = 1;
                stat->st_size = file->read().size();

                return 0;
            },
            [stat](FuseDir*) {
                stat->st_mode = S_IFDIR | 0755;
                stat->st_nlink = 2;

                return 0;
            },
            [](none) { return -ENOENT; },
        },
        entry
    );
}

int FuseFs::fuse_readdir(
    const char* path,
    void* buf,
    fuse_fill_dir_t filler,
    off_t offset,
    fuse_file_info* file_info,
    fuse_readdir_flags flags
) {
    (void)offset;
    (void)file_info;
    (void)flags;

    trace("FUSE request: readdir {}", path);

    auto [fs, lock] = get_fs_locked();

    auto* dir = fs->root->get_dir(path);

    filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_DEFAULTS);
    filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_DEFAULTS);

    const auto& entries = dir->view_entries();
    for (const auto& [name, _] : entries) {
        filler(buf, name.c_str(), nullptr, 0, FUSE_FILL_DIR_DEFAULTS);
    }

    return 0;
}

int FuseFs::fuse_read(
    const char* path,
    char* buf,
    size_t size,
    off_t offset,
    fuse_file_info* file_info
) {
    (void)file_info;

    trace("FUSE request: read {}", path);

    auto [fs, lock] = get_fs_locked();

    auto* file = fs->root->get_file(path);
    if (file == nullptr) {
        return -ENOENT;
    }

    const auto data = file->read();
    const auto* content = data.data();
    const std::size_t len = data.size();

    auto bytes_to_write = offset + size <= len ? size : len - offset;
    std::memcpy(buf, content + offset, bytes_to_write);
    return bytes_to_write;
}

int FuseFs::fuse_write(
    const char* path,
    const char* buf,
    size_t size,
    off_t offset,
    fuse_file_info* file_info
) {
    (void)file_info;

    trace("FUSE request: write {}", path);

    auto [fs, lock] = get_fs_locked();

    auto* file = fs->root->get_file(path);
    if (file == nullptr) {
        return -ENOENT;
    }

    auto span = std::as_bytes(std::span<const char>{buf, size});
    file->write(span, offset);

    return size;
}

int FuseFs::fuse_truncate(
    const char* path, off_t size, fuse_file_info* file_info
) {
    (void)file_info;

    trace("FUSE request: truncate {}", path);

    auto [fs, lock] = get_fs_locked();

    auto* file = fs->root->get_file(path);
    if (file == nullptr) {
        return -ENOENT;
    }

    file->truncate(size);

    return 0;
}
