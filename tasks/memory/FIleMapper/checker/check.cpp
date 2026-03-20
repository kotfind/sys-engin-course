#include "check.hpp"

#include "ansi.hpp"

#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>

bool check_numbers() {
    std::uint64_t last_num = 0;
    bool all_sorted = true;

    for (std::size_t num_idx = 0;; ++num_idx) {
        std::uint64_t num;
        std::cin.read((char*)&num, sizeof(std::uint64_t));

        if (std::cin.fail()) {
            break;
        }

        if (last_num > num) {
            std::cerr
                << std::format(
                       "{}Not sorted:{} nums[{:5}] = {:20}, nums[{:5}] = {:20}",
                       ANSI_BOLD_RED, ANSI_CLEAR, num_idx, last_num,
                       num_idx + 1, num)
                << std::endl;
            all_sorted = false;
        }
        last_num = num;
    }

    return all_sorted;
}
