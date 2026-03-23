#pragma once

#include "fuse_dir.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>
#include <utility>

#define FUSE_USE_VERSION 32
#include <fuse.h>

class FuseFs {
  public:
    ~FuseFs();

    static FuseFs* mount(
        const std::filesystem::path& mount_path, std::unique_ptr<FuseDir> root
    );

    bool set_file_data(std::string_view path, std::span<const std::byte> data);

    bool set_file_data_no_lock(
        std::string_view path, std::span<const std::byte> data
    );

  private:
    FuseFs(std::unique_ptr<FuseDir> root);

    static fuse_operations fuse_operations;

    // NOTE: !!! Can be called from within fuse operation handlers only
    static std::pair<FuseFs*, std::unique_lock<std::mutex>> get_fs_locked();

    static void* fuse_init(fuse_conn_info* con, fuse_config* cfg);

    static int fuse_open(const char* path, fuse_file_info* file_info);

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

    static int fuse_write(
        const char* path,
        const char* buf,
        size_t size,
        off_t offset,
        fuse_file_info* file_info
    );

    static int fuse_truncate(
        const char* path, off_t size, fuse_file_info* file_info
    );

    mutable std::mutex mutex;

    fuse_args fuse_args;
    fuse* fuse;

    std::unique_ptr<FuseDir> root;
};
