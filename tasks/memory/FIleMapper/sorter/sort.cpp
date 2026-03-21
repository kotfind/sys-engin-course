#include "sort.hpp"

#include <cstdint>
#include <format>
#include <iostream>

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

void uint64_file_sort(UInt64File& file) {
    sort_range(file, 0, file.get_items_count());
}
