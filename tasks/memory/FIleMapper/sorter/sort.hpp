#pragma once

#include <filesystem>

void uint64_file_sort(
    const std::filesystem::path& file_path, std::size_t mmap_size_limit
);
