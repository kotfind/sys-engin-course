#pragma once

#include "cpu.hpp"
#include "fuse_file.hpp"

#include <string>

/// @brief An implementation of a @ref FuseFs for `/unit{unit_id}/pram`.
class UnitProgramFile : public FuseFile {
  public:
    UnitProgramFile(Cpu* cpu, std::size_t unit_id);

    virtual ~UnitProgramFile();

    /// @brief Implements @ref FuseFs::after_close.
    void after_close(FuseFs* fs, std::string_view path) override;

    /// @brief Gets this file's expected name.
    ///
    /// That is `/unit{unit_id}/pram`.
    std::string get_file_name() const;

  private:
    /// @brief A pointer to a cpu.
    Cpu* cpu;

    /// @brief An id of a unit, managed by this file.
    std::size_t unit_id;
};
