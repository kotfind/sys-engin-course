#include "sort.hpp"

#include "log.hpp"
#include "uint64_file.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

// -------------------- MMap Algorithm --------------------

static std::size_t partition(
    UInt64File& file, std::size_t start_idx, std::size_t end_idx
) {
    auto pivot = file.get_random_item(start_idx, end_idx);
    auto i = start_idx;
    auto j = end_idx - 1;

    while (i <= j) {
        while (file[i] < pivot) {
            ++i;
        }

        while (file[j] > pivot) {
            --j;
        }

        if (i >= j) {
            break;
        }

        file.swap(i, j);
        ++i;
        --j;
    }

    return j + 1;
}

static void sort_range(
    UInt64File& file, std::size_t start_idx, std::size_t end_idx
) {
    if (end_idx - start_idx <= 1) {
        return;
    }

    auto split_at = partition(file, start_idx, end_idx);
    sort_range(file, start_idx, split_at);
    sort_range(file, split_at, end_idx);
}

static void sort_with_mmap(
    const std::filesystem::path& file_path, std::size_t mmap_size_limit
) {
    auto file = UInt64File(file_path, mmap_size_limit);
    sort_range(file, 0, file.get_items_count());
}

// -------------------- In memory algorithm --------------------

static void sort_in_memory(const std::filesystem::path& file_path) {

    // ---------- Check size ----------

    constexpr std::size_t warn_file_size = 1024 * 1024 * 1024; // 1GB
    std::size_t file_size = std::filesystem::file_size(file_path);
    if (file_size > warn_file_size) {
        warn(
            "The file is quite large ({} bytes). Let's hope, you PC won't die.",
            file_size
        );
    }

    // ---------- Read ----------

    std::vector<std::uint64_t> items;

    {
        std::ifstream fin{file_path};

        if (!fin.is_open()) {
            die("failed to open `{}` for reading", file_path.string());
        }

        while (true) {
            std::uint64_t num;
            fin.read((char*)&num, sizeof(std::uint64_t));

            if (fin.fail()) {
                break;
            }

            items.push_back(num);
        }

        fin.close();
    }

    // ---------- Sort ----------

    std::sort(std::begin(items), std::end(items));

    // ---------- Write ----------

    {
        std::ofstream fout{file_path};

        if (!fout.is_open()) {
            die("failed to open `{}` for writing", file_path.string());
        }

        for (const auto num : items) {
            fout.write((char*)&num, sizeof(std::uint64_t));
        }

        fout.close();
    }
}

// -------------------- Pick algorithm --------------------

void uint64_file_sort(
    const std::filesystem::path& file_path, std::size_t mmap_size_limit
) {
    if (mmap_size_limit < UInt64File::get_min_mmap_size_limit()) {
        warn(
            "MMap size limit is too small for the MMap algorithm "
            "(got {} bytes; min is {} bytes)",
            mmap_size_limit,
            UInt64File::get_min_mmap_size_limit()
        );
        warn("Falling back to in-memory algorithm");

        sort_in_memory(file_path);
    } else {
        info("Using MMap sorting algorithm");
        sort_with_mmap(file_path, mmap_size_limit);
    }
}
