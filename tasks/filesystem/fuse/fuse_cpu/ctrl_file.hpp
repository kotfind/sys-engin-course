#pragma once

#include "fuse_file.hpp"

class CtrlFile : public FuseFile {
  public:
    virtual ~CtrlFile();

    void write(std::span<const std::byte> data) override;

    // TODO
};
