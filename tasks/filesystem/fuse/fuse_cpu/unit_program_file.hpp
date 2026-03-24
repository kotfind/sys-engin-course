#pragma once

#include "cpu.hpp"
#include "fuse_file.hpp"

#include <string>

class UnitProgramFile : public FuseFile {
  public:
    UnitProgramFile(Cpu* cpu, std::size_t unit_id);

    virtual ~UnitProgramFile();

    void after_close() override;

    std::string get_file_name() const;

  private:
    Cpu* cpu;
    std::size_t unit_id;
};
