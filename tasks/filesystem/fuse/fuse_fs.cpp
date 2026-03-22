#include "fuse_fs.hpp"
#include "fuse_dir.hpp"
#include "fuse_file.hpp"
#include "helpers.hpp"
#include "log.hpp"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
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
    .open = FuseFs::fuse_open,
    .read = FuseFs::fuse_read,
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
    std::filesystem::path mount_path_, std::unique_ptr<FuseDir> root
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

bool FuseFs::set_file_data(
    std::string_view path, std::span<const std::byte> data
) {
    std::unique_lock lock{this->mutex};
    return set_file_data_no_lock(path, data);
}

bool FuseFs::set_file_data_no_lock(
    std::string_view path, std::span<const std::byte> data
) {
    info("Setting data for file {}", path);

    auto entry = this->root->get_entry(path);
    auto* file = std::visit(
        overloads{
            [](FuseFile* file) { return file; },
            [](FuseDir*) {
                error("Failed to write to a file: this is a dir");
                return (FuseFile*)nullptr;
            },
            [](none) {
                error("Faield to write to a file: path does not exist");
                return (FuseFile*)nullptr;
            },
        },
        entry
    );
    if (file == nullptr) {
        return false;
    }

    file->set_data_unsafe(data);
    fuse_invalidate_path(this->fuse, path.data());

    success("Updated data for file {}", path);
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
    (void)path;

    // We invalidate cache manually on each file modification anyway.

    file_info->keep_cache = 1;

    return 0;
}

int FuseFs::fuse_getattr(
    const char* path, struct stat* stat, fuse_file_info* file_info
) {
    (void)file_info;

    auto [fs, lock] = get_fs_locked();

    info("FUSE request: getattr {}", path);

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

    info("FUSE request: readdir {}", path);

    auto [fs, lock] = get_fs_locked();

    auto entry = fs->root->get_entry(path);
    auto* dir = std::visit(
        overloads{
            [](FuseDir* dir) { return dir; },
            [](FuseFile*) {
                error("Failed to readdir: this is a regular file");
                return (FuseDir*)nullptr;
            },
            [](none) {
                error("Faield to readdir: path does not exist");
                return (FuseDir*)nullptr;
            },
        },
        entry
    );
    if (dir == nullptr) {
        return -ENOENT;
    }

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

    info("FUSE request: read {}", path);

    auto [fs, lock] = get_fs_locked();

    auto entry = fs->root->get_entry(path);
    auto* file = std::visit(
        overloads{
            [](FuseFile* file) { return file; },
            [](FuseDir*) {
                error("Failed to read: this is a dir");
                return (FuseFile*)nullptr;
            },
            [](none) {
                error("Faield to read: path does not exist");
                return (FuseFile*)nullptr;
            },
        },
        entry
    );
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
