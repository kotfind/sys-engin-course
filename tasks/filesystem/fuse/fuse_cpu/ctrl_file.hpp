#pragma once

#include "cpu.hpp"
#include "fuse_file.hpp"

#include <string_view>

/// @brief An implementation of a @ref FuseFs for `/ctrl`.
class CtrlFile : public FuseFile {
  public:
    CtrlFile(Cpu* cpu);

    virtual ~CtrlFile();

    /// @brief An implementation of @ref FuseFile::after_close.
    void after_close(FuseFs* fs, std::string_view path) override;

    /// @brief Updates the `read_data` buffer.
    void recalc_read_data(FuseFs* fs, std::string_view path);

    /// @brief Get this file name (`/ctrl`).
    std::string_view get_file_name() const;

  private:
    /// @brief A pointer to the cpu.
    Cpu* cpu;
};
