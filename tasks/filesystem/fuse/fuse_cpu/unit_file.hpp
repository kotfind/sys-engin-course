#pragma once

#include "fuse_file.hpp"

class UnitFile : public FuseFile {
  public:
    virtual ~UnitFile();

    void write(std::span<const std::byte> data) override;

    // TODO
};
