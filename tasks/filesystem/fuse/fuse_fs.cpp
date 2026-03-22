#include "fuse_fs.hpp"
#include "log.hpp"

#include <cstring>
#include <memory>
#include <mutex>
#include <sys/types.h>
#include <thread>

#define FUSE_USE_VERSION 32
#include <fuse.h>
#include <fuse_opt.h>

#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
struct fuse_operations FuseFs::fuse_operations = {
    .getattr = FuseFs::fuse_getattr,
    .read = FuseFs::fuse_read,
    .readdir = FuseFs::fuse_readdir,
};

FuseFs::FuseFs() : fuse(nullptr) {
}

FuseFs::~FuseFs() {
    if (this->fuse != nullptr) {
        fuse_unmount(this->fuse);
        fuse_destroy(this->fuse);
    }
}

FuseFs* FuseFs::mount(std::string_view mountpath) {
    auto fs = std::unique_ptr<FuseFs>(new FuseFs());

    fuse_args args;
    {
        static std::string dummy = "dummy";
        static char* dummy_argv[] = {dummy.data(), NULL};
        args = FUSE_ARGS_INIT(1, dummy_argv);
    }

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

int FuseFs::fuse_getattr(
    const char* path, struct stat* stat, fuse_file_info* info
) {
    auto* fs = (FuseFs*)fuse_get_context()->private_data;

    std::lock_guard lock(fs->mutex);
    return fs->get_attr(path, stat, info);
}

int FuseFs::fuse_readdir(
    const char* path,
    void* buf,
    fuse_fill_dir_t filler,
    off_t offset,
    fuse_file_info* info,
    fuse_readdir_flags flags
) {
    auto* fs = (FuseFs*)fuse_get_context()->private_data;

    std::lock_guard lock(fs->mutex);
    return fs->read_dir(path, buf, filler, offset, info, flags);
}

int FuseFs::fuse_read(
    const char* path, char* buf, size_t size, off_t offset, fuse_file_info* info
) {
    auto* fs = (FuseFs*)fuse_get_context()->private_data;

    std::lock_guard lock(fs->mutex);
    return fs->read_file(path, buf, size, offset, info);
}

int FuseFs::get_attr(
    const char* path, struct stat* stat, fuse_file_info* info
) const {
    (void)info;

    std::memset(stat, 0, sizeof(struct stat));

    if (std::strcmp(path, "/") == 0) {
        stat->st_mode = S_IFDIR | 0755;
        stat->st_nlink = 2;

        return 0;
    }

    if (std::strcmp(path, "/hello.txt") == 0) {
        stat->st_mode = S_IFREG | 0444;
        stat->st_nlink = 1;
        stat->st_size = 13;

        return 0;
    }

    return -ENOENT;
}

int FuseFs::read_dir(
    const char* path,
    void* buf,
    fuse_fill_dir_t filler,
    off_t offset,
    fuse_file_info* info,
    fuse_readdir_flags flags
) const {
    (void)offset;
    (void)info;
    (void)flags;

    if (std::strcmp(path, "/") != 0) {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    filler(buf, "hello.txt", NULL, 0, FUSE_FILL_DIR_DEFAULTS);

    return 0;
}

int FuseFs::read_file(
    const char* path, char* buf, size_t size, off_t offset, fuse_file_info* info
) const {
    (void)info;

    if (std::strcmp(path, "/hello.txt") != 0) {
        return -ENOENT;
    }

    const char* content = "Hello, world!";
    size_t len = std::strlen(content);

    if ((std::size_t)offset < len) {
        if (offset + size > len) {
            size = len - offset;
        }
        std::memcpy(buf, content + offset, size);
    } else {
        size = 0;
    }

    return size;
}
