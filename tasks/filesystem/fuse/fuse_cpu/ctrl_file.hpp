#pragma once

#include "fuse_file.hpp"

class CtrlFile : public FuseFile {
  public:
    virtual ~CtrlFile();

    void after_close() override;
};
