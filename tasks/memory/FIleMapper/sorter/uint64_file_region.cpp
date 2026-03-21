#include "uint64_file_region.hpp"

#include "ansi.hpp"

#include <cerrno>
#include <cstring>
#include <format>
#include <iostream>
#include <random>
#include <sys/mman.h>
#include <unistd.h>

static std::mt19937_64 rnd{std::random_device{}()};

UInt64FileRegion::UInt64FileRegion(
    int fd, std::size_t start_idx, std::size_t end_idx
)
    : start_idx(start_idx), end_idx(end_idx) {

    auto start_offset = start_idx * sizeof(std::uint64_t);
    auto end_offset = end_idx * sizeof(std::uint64_t);
    auto bytes_size = end_offset - start_offset;

    this->items = (std::uint64_t*)mmap(
        NULL, bytes_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start_offset
    );

    if (this->items == MAP_FAILED) {
        std::cerr << ANSI_BOLD_RED << "mmap failed: " << strerror(errno)
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }
}

UInt64FileRegion::UInt64FileRegion(UInt64FileRegion&& other)
    : items(other.items), start_idx(other.start_idx), end_idx(other.end_idx) {
    other.items = nullptr;
}

UInt64FileRegion& UInt64FileRegion::operator=(UInt64FileRegion&& other) {
    this->unmap();

    this->items = other.items;
    this->start_idx = other.start_idx;
    this->end_idx = other.end_idx;

    other.items = nullptr;

    return *this;
};

UInt64FileRegion::~UInt64FileRegion() {
    this->unmap();
}

void UInt64FileRegion::unmap() {
    auto start_offset = start_idx * sizeof(std::uint64_t);
    auto end_offset = end_idx * sizeof(std::uint64_t);
    auto bytes_size = end_offset - start_offset;

    if (munmap(this->items, bytes_size) == -1) {
        std::cerr << ANSI_BOLD_RED << "munmap failed: " << strerror(errno)
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }
}

std::size_t UInt64FileRegion::get_start_idx() const {
    return this->start_idx;
}

std::size_t UInt64FileRegion::get_end_idx() const {
    return this->end_idx;
}

bool UInt64FileRegion::is_in_range(std::size_t abs_idx) const {
    return this->start_idx <= abs_idx && abs_idx < this->end_idx;
}

std::uint64_t& UInt64FileRegion::operator[](std::size_t abs_idx) {
    this->assert_in_range(abs_idx);
    auto rel_idx = abs_idx - this->start_idx;
    return this->items[rel_idx];
}

const std::uint64_t& UInt64FileRegion::operator[](std::size_t abs_idx) const {
    this->assert_in_range(abs_idx);
    auto rel_idx = abs_idx - this->start_idx;
    return this->items[rel_idx];
}

void UInt64FileRegion::assert_in_range(std::size_t abs_idx) const {
    if (this->is_in_range(abs_idx)) {
        return;
    }

    std::cerr << ANSI_BOLD_RED
              << std::format(
                     "index {} is not in range [{}, {})",
                     abs_idx,
                     this->start_idx,
                     this->end_idx
                 )
              << ANSI_CLEAR << std::endl;
    exit(1);
}

std::size_t UInt64FileRegion::get_start_idx_alignment() {
    auto page_size = sysconf(_SC_PAGESIZE);

    if (page_size == -1) {
        std::cerr << ANSI_BOLD_RED
                  << std::format(
                         "failed to get a page size {}", strerror(errno)
                     )
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }

    return page_size / sizeof(std::uint64_t);
}
