#pragma once

#include "cpu.hpp"
#include "fuse_file.hpp"

#include <cstddef>
#include <string>
#include <string_view>

class UnitDataFile : public FuseFile {
  public:
    UnitDataFile(Cpu* cpu, std::size_t unit_id);

    virtual ~UnitDataFile();

    static std::string get_file_name_static(std::size_t unit_id);

    void after_close(FuseFs* fs, std::string_view path) override;

    std::string get_file_name() const;

  private:
    Cpu* cpu;

    std::size_t unit_id;
};
