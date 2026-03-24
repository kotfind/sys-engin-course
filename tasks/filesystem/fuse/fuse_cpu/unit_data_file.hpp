#pragma once

#include "cpu.hpp"
#include "fuse_file.hpp"

#include <cstddef>
#include <string>
#include <string_view>

/// @brief An implementation of a @ref FuseFs for `/unit{unit_id}/lram`.
class UnitDataFile : public FuseFile {
  public:
    UnitDataFile(Cpu* cpu, std::size_t unit_id);

    virtual ~UnitDataFile();

    /// @brief The same as @ref UnitDataFile::get_file_name, but static.
    static std::string get_file_name_static(std::size_t unit_id);

    /// @brief Implements @ref FuseFs::after_close.
    void after_close(FuseFs* fs, std::string_view path) override;

    /// @brief Gets this file's expected name.
    ///
    /// That is `/unit{unit_id}/lram`.
    std::string get_file_name() const;

  private:
    /// @brief A pointer to a cpu.
    Cpu* cpu;

    /// @brief An id of a unit, managed by this file.
    std::size_t unit_id;
};
