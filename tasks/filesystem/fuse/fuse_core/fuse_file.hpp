#pragma once

#include <cstddef>
#include <span>
#include <vector>

class FuseFs;

class FuseFile {
  public:
    virtual ~FuseFile();

    std::span<const std::byte> read() const;

    void write(std::span<const std::byte> data, std::size_t offset);

    void truncate(std::size_t size);

    virtual void after_close() = 0;

  protected:
    std::span<const std::byte> get_read_data() const;

    std::span<const std::byte> get_write_data() const;

  private:
    std::vector<std::byte> read_data;

    std::vector<std::byte> write_data;
};
