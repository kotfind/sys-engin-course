#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

struct Args {
    std::filesystem::path file_path;
    std::size_t mmap_size_limit;
};

Args parse_args(int argc, char** argv);

void show_usage(const std::string argv0, bool is_error);
