#include "gen.hpp"

#include <iostream>
#include <random>

std::mt19937_64 rnd{std::random_device{}()};

void print_random_numbers(std::size_t num_count) {
    std::uniform_int_distribution<uint64_t> dist;

    for (std::size_t i = 0; i < num_count; ++i) {
        auto num = dist(rnd);

        std::cout.write((const char*)&num, sizeof(uint64_t));
    }
}
