/// @file
///
/// @brief A fuse filesystem.

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

/// @brief A fuse filesystem.
///
/// To integrate with fuse api, on mounting this class sets it's pointer
/// as a private_data in a fuse context, which later allows to access it from
/// the fuse operation handlers.
class FuseFs {
  public:
    ~FuseFs();

    /// @brief Tries to mount a filesystem.
    /// @return A new instance or nullptr if fails.
    static FuseFs* mount(
        const std::filesystem::path& mount_path, std::unique_ptr<FuseDir> root
    );

    /// @brief Invalidates kernel's file cache.
    ///
    /// NOTE: !!! Don't call from within a writing operation: will deadlock
    bool invalidate_path(std::string_view path);

    /// @brief Sets a file's `read_data` buffer.
    ///
    /// NOTE: !!! Don't call from within a writing operation: will deadlock
    ///
    /// Refer to FuseFile class documentation for more information.
    bool set_file_read_data(
        std::string_view path, std::span<const std::byte> data
    );

  private:
    FuseFs(std::unique_ptr<FuseDir> root);

    static fuse_operations fuse_operations;

    /// @brief Gets an instance of this class from fuse context and locks it.
    ///
    /// NOTE: !!! Can be called from within fuse operation handlers only.
    static std::pair<FuseFs*, std::unique_lock<std::mutex>> get_fs_locked();

    /// @brief Implements fuse **init** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static void* fuse_init(fuse_conn_info* con, fuse_config* cfg);

    /// @brief Implements fuse **open** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static int fuse_open(const char* path, fuse_file_info* file_info);

    /// @brief Implements fuse **flush** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static int fuse_flush(const char* path, struct fuse_file_info* file_info);

    /// @brief Implements fuse **getattr** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static int fuse_getattr(
        const char* path, struct stat* stat, fuse_file_info* info
    );

    /// @brief Implements fuse **readdir** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static int fuse_readdir(
        const char* path,
        void* buf,
        fuse_fill_dir_t filler,
        off_t offset,
        fuse_file_info* info,
        fuse_readdir_flags flags
    );

    /// @brief Implements fuse **read** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static int fuse_read(
        const char* path,
        char* buf,
        size_t size,
        off_t offset,
        fuse_file_info* info
    );

    /// @brief Implements fuse **write** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static int fuse_write(
        const char* path,
        const char* buf,
        size_t size,
        off_t offset,
        fuse_file_info* file_info
    );

    /// @brief Implements fuse **truncate** operation.
    ///
    /// Refer to the class-level documentation for more information.
    static int fuse_truncate(
        const char* path, off_t size, fuse_file_info* file_info
    );

    /// @brief A global filesystem mutex.
    mutable std::mutex mutex;

    /// @brief Fuse arguments.
    ///
    /// This field is only stored to be deallocated in the destructor.
    fuse_args fuse_args;

    /// @brief A fuse filesystem handle.
    fuse* fuse;

    /// @brief The filesystem's root directory.
    std::unique_ptr<FuseDir> root;
};
