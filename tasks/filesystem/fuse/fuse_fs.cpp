#include "fuse_fs.hpp"
#include "fuse_dir.hpp"
#include "fuse_entry.hpp"
#include "fuse_file.hpp"
#include "helpers.hpp"
#include "log.hpp"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
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
    .read = FuseFs::fuse_read,
    .readdir = FuseFs::fuse_readdir,
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

std::pair<FuseFs*, std::unique_lock<std::mutex>> FuseFs::get_fs_locked() {
    auto* ctx = fuse_get_context();
    assert(ctx != nullptr);

    auto* fs = (FuseFs*)ctx->private_data;
    std::unique_lock lock{fs->mutex};

    return {fs, std::move(lock)};
}

int FuseFs::fuse_open(const char* path, fuse_file_info* file_info) {
    auto [fs, lock] = get_fs_locked();

    info("FUSE request: open {}", path);

    auto entry = fs->root->get_entry(path);
    return std::visit(
        overloads{
            [file_info](FuseDir*) {
                error("Failed to open: this is a directory");
                file_info->fh = 0;
                return -EISDIR;
            },
            [file_info](FuseFile* file) {
                file_info->fh = (uint64_t)file;
                return 0;
            },
            [file_info](none) {
                error("Faield to open: path does not exist");
                file_info->fh = 0;
                return -ENOENT;
            },
        },
        entry
    );
}

int FuseFs::fuse_opendir(const char* path, fuse_file_info* file_info) {
    auto [fs, lock] = get_fs_locked();

    info("FUSE request: opendir {}", path);

    auto entry = fs->root->get_entry(path);
    return std::visit(
        overloads{
            [file_info](FuseDir* dir) {
                file_info->fh = (uint64_t)dir;
                return 0;
            },
            [file_info](FuseFile*) {
                error("Failed to opendir: this is a regular file");
                file_info->fh = 0;
                return -ENOTDIR;
            },
            [file_info](none) {
                error("Faield to opendir: path does not exist");
                file_info->fh = 0;
                return -ENOENT;
            },
        },
        entry
    );
}

int FuseFs::fuse_getattr(
    const char* path, struct stat* stat, fuse_file_info* file_info
) {
    auto [fs, lock] = get_fs_locked();

    info("FUSE request: getattr {}", path);

    auto* entry = (FuseEntry*)file_info->fh;
    if (entry == nullptr) {
        return -ENOENT;
    }

    switch (entry->get_entry_type()) {
    case FuseEntryType::File:
        stat->st_mode = S_IFREG | 0444;
        stat->st_nlink = 1;
        stat->st_size = ((FuseFile*)entry)->get_data().size();
        break;

    case FuseEntryType::Dir:
        stat->st_mode = S_IFDIR | 0755;
        stat->st_nlink = 2;
        break;
    }

    return 0;
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
    (void)flags;

    info("FUSE request: readdir {}", path);

    auto [fs, lock] = get_fs_locked();

    auto* dir = (FuseDir*)file_info->fh;
    if (dir == nullptr) {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);

    const auto& entries = dir->view_entries();
    for (const auto& [name, _] : entries) {
        filler(buf, name.c_str(), NULL, 0, FUSE_FILL_DIR_DEFAULTS);
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
    info("FUSE request: read {}", path);

    auto [fs, lock] = get_fs_locked();

    auto* file = (FuseFile*)file_info->fh;
    if (file == nullptr) {
        return -ENOENT;
    }

    const auto data = file->get_data();
    const auto* content = data.data();
    const std::size_t len = data.size();

    auto bytes_to_write = offset + size <= len ? size : len - offset;
    std::memcpy(buf, content + offset, bytes_to_write);
    return bytes_to_write;
}
