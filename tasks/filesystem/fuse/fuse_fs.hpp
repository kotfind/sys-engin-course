#pragma once

#include "fuse_dir.hpp"

#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#define FUSE_USE_VERSION 32
#include <fuse.h>

class FuseFs {
  public:
    ~FuseFs();

    static FuseFs* mount(
        std::string_view mountpath, std::unique_ptr<FuseDir> root
    );

  private:
    FuseFs(std::unique_ptr<FuseDir> root);

    static struct fuse_operations fuse_operations;

    // NOTE: !!! Can be called from within fuse operation handlers only
    static std::pair<FuseFs*, std::unique_lock<std::mutex>> get_fs_locked();

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

    mutable std::mutex mutex;

    struct fuse* fuse;

    std::unique_ptr<FuseDir> root;
};
