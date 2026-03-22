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

    virtual void write(
        std::span<const std::byte> data, std::string_view path, FuseFs* fuse_fs
    ) = 0;

  private:
    void set_data_unsafe(std::span<const std::byte> data);

    std::vector<std::byte> data;

    friend FuseFs;
};

class SimpleFuseFile : public FuseFile {
  public:
    virtual ~SimpleFuseFile();

    void write(
        std::span<const std::byte> data, std::string_view path, FuseFs* fuse_fs
    ) override;
};
