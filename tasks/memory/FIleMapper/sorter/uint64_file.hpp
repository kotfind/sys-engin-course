#pragma once

#include "uint64_file_region.hpp"

#include <filesystem>
#include <vector>

class UInt64File {
  public:
    UInt64File(
        const std::filesystem::path& file_path, std::size_t mmap_size_limit
    );

    ~UInt64File();

    std::uint64_t& operator[](std::size_t abs_idx);

    std::size_t get_items_cont() const;

  private:
    using last_used_counter_t = std::size_t;

    const std::size_t max_regions = 4;

    int fd;

    std::size_t items_count;

    last_used_counter_t last_used_counter = 0;

    // max_size = `this->max_regions`
    //
    // region start_idx's are `this->max_region_size`-aligned
    std::vector<std::pair<UInt64FileRegion, last_used_counter_t>> regions;

    // = `mmap_size_limit / sizeof(std::uint64_t) / this->cached_regions_count`
    std::size_t max_region_size;
};
