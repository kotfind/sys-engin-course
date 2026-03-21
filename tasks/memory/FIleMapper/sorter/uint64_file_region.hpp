#pragma once

#include <cstdint>
#include <filesystem>

/// A `mmap`-ed region of a file, that contains unsigned 64-bit integers.
class UInt64FileRegion {
  public:
    UInt64FileRegion(int fd, std::size_t start_idx, std::size_t end_idx);

    UInt64FileRegion(UInt64FileRegion&&);

    // Cannot copy, as the destructor frees mapped memory.
    UInt64FileRegion(const UInt64FileRegion&) = delete;

    // Cannot copy, as the destructor frees mapped memory.
    UInt64FileRegion& operator=(const UInt64FileRegion&) = delete;

    UInt64FileRegion& operator=(UInt64FileRegion&&);

    ~UInt64FileRegion();

    std::size_t get_start_idx() const;

    std::size_t get_end_idx() const;

    bool is_in_range(std::size_t abs_idx) const;

    std::uint64_t& operator[](std::size_t abs_idx);

    const std::uint64_t& operator[](std::size_t abs_idx) const;

    static std::size_t get_start_idx_alignment();

  private:
    void assert_in_range(std::size_t abs_idx) const;

    void unmap();

    std::uint64_t* items;

    std::size_t start_idx;
    std::size_t end_idx;
};
