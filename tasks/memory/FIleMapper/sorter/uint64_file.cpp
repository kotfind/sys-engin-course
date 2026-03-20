#include "uint64_file.hpp"

#include "ansi.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <format>
#include <iostream>
#include <numeric>
#include <unistd.h>

UInt64File::UInt64File(
    const std::filesystem::path& file_path, std::size_t mmap_size_limit
) {
    this->fd = open(file_path.c_str(), O_RDWR);

    if (this->fd == -1) {
        std::cerr << ANSI_BOLD_RED
                  << std::format(
                         "failed to open `{}`: {}",
                         file_path.c_str(),
                         strerror(errno)
                     )
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }

    ;
    auto min_mmap_size_limit =
        sizeof(std::uint64_t) *
        std::max(
            this->max_regions, UInt64FileRegion::get_start_idx_alignment()
        );

    if (mmap_size_limit < min_mmap_size_limit) {
        std::cerr << ANSI_BOLD_RED
                  << std::format(
                         "mmap size limits less than {} bytes are not "
                         "supported, please specify a higher value (got a "
                         "value of {} bytes)",
                         min_mmap_size_limit,
                         mmap_size_limit,
                         strerror(errno)
                     )
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }

    this->max_region_size =
        mmap_size_limit / sizeof(std::uint64_t) / this->max_regions;
    this->max_region_size =
        this->max_region_size -
        (this->max_region_size % UInt64FileRegion::get_start_idx_alignment());
    assert(this->max_region_size != 0);

    this->regions.reserve(this->max_regions);

    this->items_count =
        std::filesystem::file_size(file_path) / sizeof(std::uint64_t);
}

UInt64File::~UInt64File() {
    if (this->fd) {
        if (close(this->fd) == -1) {
            std::cerr << ANSI_BOLD_RED
                      << std::format(
                             "failed to close file: {}", strerror(errno)
                         )
                      << ANSI_CLEAR << std::endl;
            exit(1);
        }
    }
}

std::size_t UInt64File::get_items_cont() const {
    return this->items_count;
}

std::uint64_t& UInt64File::operator[](std::size_t abs_idx) {
    // ---------- Checks ----------

    if (abs_idx >= this->items_count) {
        std::cerr << ANSI_BOLD_RED
                  << std::format(
                         "index {} is not in range: [0, {}): {}",
                         abs_idx,
                         this->items_count,
                         strerror(errno)
                     )
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }

    // ---------- Try find in cache ----------

    auto start_idx = abs_idx - (abs_idx % this->max_region_size);

    ++this->last_used_counter;

    for (auto& [region, last_used] : this->regions) {
        if (region.get_start_idx() == start_idx) {
            last_used += this->last_used_counter;
            return region[abs_idx];
        }
    }

    // ---------- Try load region without replacing ----------

    auto end_idx =
        std::min(start_idx + this->max_region_size, this->items_count);

    auto new_region = std::make_pair(
        UInt64FileRegion(this->fd, start_idx, end_idx), this->last_used_counter
    );

    if (this->regions.size() < this->max_regions) {
        this->regions.push_back(std::move(new_region));
        return this->regions.back().first[abs_idx];
    }

    // ---------- Load region with replacing ----------

    std::size_t oldest_region_idx = 0;
    for (std::size_t region_idx = 0; region_idx < this->regions.size();
         ++region_idx) {
        if (this->regions[region_idx].second <
            this->regions[oldest_region_idx].second) {
            oldest_region_idx = region_idx;
        }
    }

    this->regions[oldest_region_idx] = std::move(new_region);
    return this->regions[oldest_region_idx].first[abs_idx];
}
