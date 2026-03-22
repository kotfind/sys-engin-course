#pragma once

#include <mutex>
#define FUSE_USE_VERSION 32
#include <fuse.h>

#include <string_view>

class FuseFs {
  public:
    ~FuseFs();

    static FuseFs* mount(std::string_view mountpath);

  private:
    FuseFs();

    static struct fuse_operations fuse_operations; // TODO: define me

    static int fuse_getattr(
        const char* path, struct stat* stat, fuse_file_info* info
    );

    static int fuse_readdir(
        const char* path,
        void* buf,
        fuse_fill_dir_t filler,
        off_t offset,
        fuse_file_info* info,
        fuse_readdir_flags flags
    );

    static int fuse_read(
        const char* path,
        char* buf,
        size_t size,
        off_t offset,
        fuse_file_info* info
    );

    int get_attr(
        const char* path, struct stat* stat, fuse_file_info* info
    ) const;

    int read_dir(
        const char* path,
        void* buf,
        fuse_fill_dir_t filler,
        off_t offset,
        fuse_file_info* info,
        fuse_readdir_flags flags
    ) const;

    int read_file(
        const char* path,
        char* buf,
        size_t size,
        off_t offset,
        fuse_file_info* info
    ) const;

    mutable std::mutex mutex;

    struct fuse* fuse;
};
