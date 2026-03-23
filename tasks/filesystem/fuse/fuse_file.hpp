#pragma once

#include <cstddef>
#include <span>
#include <vector>

class FuseFs;

class FuseFile {
  public:
    virtual ~FuseFile();

    std::span<const std::byte> read() const;

    virtual void write(std::span<const std::byte> data) = 0;

  protected:
    void set_data_no_invalidate(std::span<const std::byte> data);

  private:
    std::vector<std::byte> data;

    friend FuseFs;
};

class SimpleFuseFile : public FuseFile {
  public:
    virtual ~SimpleFuseFile();

    void write(std::span<const std::byte> data) override;
};
