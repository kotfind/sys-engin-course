#pragma once

#include <cstddef>
#include <filesystem>

struct Args {
    std::size_t unit_count;

    std::filesystem::path mount_point;
};

Args parse_args(int argc, char** argv);
