#pragma once

#include "fuse_fs.hpp"
#include "unit.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>

class FuseCpu {
  public:
    static FuseCpu* create(
        const std::filesystem::path& mount_path, std::size_t unit_count
    );

  private:
    FuseCpu(
        std::unique_ptr<FuseFs> fs, std::vector<std::unique_ptr<Unit>> units
    );

    std::unique_ptr<FuseFs> fs;
    std::vector<std::unique_ptr<Unit>> units;
};
