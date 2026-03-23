#pragma once

#include "fuse_file.hpp"

class UnitFile : public FuseFile {
  public:
    virtual ~UnitFile();

    void after_close() override;

    // TODO
};
