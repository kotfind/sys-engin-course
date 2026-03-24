#pragma once

#include "cpu.hpp"
#include "fuse_file.hpp"

#include <string_view>

class CtrlFile : public FuseFile {
  public:
    CtrlFile(Cpu* cpu);

    virtual ~CtrlFile();

    void after_close() override;

    std::string_view get_file_name() const;

  private:
    Cpu* cpu;
};
