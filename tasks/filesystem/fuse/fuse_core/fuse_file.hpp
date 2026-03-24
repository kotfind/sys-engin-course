#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

class FuseFs;

class FuseFile {
  public:
    virtual ~FuseFile();

    std::span<const std::byte> read() const;

    void write(std::span<const std::byte> data, std::size_t offset);

    void truncate(std::size_t size);

    virtual void after_close(FuseFs* fs, std::string_view path) = 0;

    // NOTE: !!! Don't call from within a writing operation: will deadlock
    void set_read_data(
        std::span<const std::byte> data, FuseFs* fs, std::string_view pat
    );

    void set_read_data_no_invalidate(std::span<const std::byte> data);

  protected:
    std::span<const std::byte> get_read_data() const;

    std::span<const std::byte> get_write_data() const;

    std::vector<std::byte> move_write_data();

  private:
    std::vector<std::byte> read_data;

    std::vector<std::byte> write_data;
};
