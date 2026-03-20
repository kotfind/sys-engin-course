#include "ansi.hpp"
#include "args.hpp"
#include "gen.hpp"

#include <format>
#include <iostream>

int main(int argc, char** argv) {
    auto num_count = parse_args(argc, argv).num_count;

    std::cerr << ANSI_BOLD
              << std::format("Going to write {} numbers to stdout...",
                             num_count)
              << ANSI_CLEAR << std::endl;

    print_random_numbers(num_count);

    std::cerr << ANSI_BOLD
              << std::format("Wrote {} numbers to stdout", num_count)
              << ANSI_CLEAR << std::endl;
}
