#pragma once

#include "cpu.hpp"
#include "fuse_fs.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>

class FuseCpu {
  public:
    static FuseCpu* create(
        const std::filesystem::path& mount_path, std::size_t unit_count
    );

  private:
    FuseCpu(std::unique_ptr<FuseFs> fs, std::unique_ptr<Cpu> cpu);

    std::unique_ptr<FuseFs> fs;

    std::unique_ptr<Cpu> cpu;
};
