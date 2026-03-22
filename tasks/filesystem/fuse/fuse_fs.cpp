#include "fuse_fs.hpp"
#include "fuse_dir.hpp"
#include "fuse_file.hpp"
#include "helpers.hpp"
#include "log.hpp"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
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
    if (this->fuse != nullptr) {
        fuse_unmount(this->fuse);
        fuse_destroy(this->fuse);
    }
}

FuseFs* FuseFs::mount(
    std::string_view mountpath, std::unique_ptr<FuseDir> root
) {
    auto fs = std::unique_ptr<FuseFs>(new FuseFs(std::move(root)));

    fuse_args args;
    {
        static std::string dummy = "dummy";
        static char* dummy_argv[] = {dummy.data(), NULL};
        args = FUSE_ARGS_INIT(1, dummy_argv);
    }

    // WARN: the following line causes a memory leak.
    // idk why: FuseFs' destructor is properly called
    fs->fuse =
        fuse_new(&args, &fuse_operations, sizeof(fuse_operations), fs.get());
    if (fs->fuse == nullptr) {
        error("Failed to init FUSE");
        return nullptr;
    }

    if (fuse_mount(fs->fuse, mountpath.data()) != 0) {
        error("Failed to mount FUSE");
        return nullptr;
    }

    {
        auto* fuse = fs->fuse;
        std::thread([fuse] { fuse_loop(fuse); }).detach();
    }

    return fs.release();
}

std::pair<FuseFs*, std::unique_lock<std::mutex>> FuseFs::get_fs_locked() {
    auto* ctx = fuse_get_context();
    assert(ctx != nullptr);

    auto* fs = (FuseFs*)ctx->private_data;
    std::unique_lock lock{fs->mutex};

    return {fs, std::move(lock)};
}

int FuseFs::fuse_getattr(
    const char* path, struct stat* stat, fuse_file_info* info
) {
    (void)info;

    auto [fs, lock] = get_fs_locked();

    auto entry = fs->root->get_entry(path);
    return std::visit(
        overloads{
            [stat](FuseFile* file) {
                stat->st_mode = S_IFREG | 0444;
                stat->st_nlink = 1;
                stat->st_size = file->get_data().size();

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
    fuse_file_info* info,
    fuse_readdir_flags flags
) {
    (void)offset;
    (void)info;
    (void)flags;

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

    filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);

    const auto& entries = dir->view_entries();
    for (const auto& [name, _] : entries) {
        filler(buf, name.c_str(), NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    }

    return 0;
}

int FuseFs::fuse_read(
    const char* path, char* buf, size_t size, off_t offset, fuse_file_info* info
) {
    (void)info;

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
                error("Faield to rad: path does not exist");
                return (FuseFile*)nullptr;
            },
        },
        entry
    );
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
