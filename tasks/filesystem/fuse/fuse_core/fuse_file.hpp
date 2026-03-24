#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

class FuseFs;

/// @brief A file.
///
/// It holds two buffers: `read_data` and `write_data`.
/// Fuse uses `read_data` for read operations and `write_data` for write and
/// truncate operations. Synchronise the buffers manually if required.
class FuseFile {
  public:
    virtual ~FuseFile();

    /// @brief Implements fuse read operation.
    std::span<const std::byte> read() const;

    /// @brief Implements fuse write operation.
    ///
    /// NOTE: !!! Does not invalidate kernel's cache. This is intentional as
    /// doing it would cause a deadlock.
    void write(std::span<const std::byte> data, std::size_t offset);

    /// @brief Implements fuse truncate operation.
    void truncate(std::size_t size);

    /// @brief Is called after fuse close call.
    virtual void after_close(FuseFs* fs, std::string_view path) = 0;

    /// @brief Set's the read_data buffer.
    ///
    /// NOTE: !!! Don't call from within a writing operation: will deadlock.
    void set_read_data(
        std::span<const std::byte> data, FuseFs* fs, std::string_view pat
    );

    /// @brief Set's the read_data buffer without invalidating kernel's cache.
    ///
    /// Is useful, when implementing a write operation.
    void set_read_data_no_invalidate(std::span<const std::byte> data);

  protected:
    /// @brief Gets a `read_data` buffer.
    ///
    /// Refer to the class-level documentation for more info.
    std::span<const std::byte> get_read_data() const;

    /// @brief Gets a `write_data` buffer.
    ///
    /// Refer to the class-level documentation for more info.
    std::span<const std::byte> get_write_data() const;

    /// @brief Moves a `write_data` buffer.
    ///
    /// Moving is done by swapping the data with an empty vector.
    /// Thus the internal data won't be in the invalid state (but will be
    /// empty).
    std::vector<std::byte> move_write_data();

  private:
    /// @brief A `read_data` buffer.
    ///
    /// Refer to the class-level documentation for more info.
    std::vector<std::byte> read_data;

    /// @brief A `read_data` buffer.
    ///
    /// Refer to the class-level documentation for more info.
    std::vector<std::byte> write_data;
};
