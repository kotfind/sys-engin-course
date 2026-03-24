#pragma once

#include "cpu.hpp"
#include "fuse_fs.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>

/// @brief A bridge between a @ref Cpu and a @ref FuseFs.
///
/// This class brings two parts of the application together.
class FuseCpu {
  public:
    /// @brief Tries to create a new instance of this class.
    /// @return A pointer to a new instance or nullptr if fails.
    static FuseCpu* create(
        const std::filesystem::path& mount_path, std::size_t unit_count
    );

  private:
    FuseCpu(std::unique_ptr<FuseFs> fs, std::unique_ptr<Cpu> cpu);

    /// @brief A pointer to a filesystem.
    std::unique_ptr<FuseFs> fs;

    /// @brief A pointer to a cpu.
    std::unique_ptr<Cpu> cpu;
};
