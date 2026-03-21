#include "args.hpp"

#include "ansi.hpp"
#include "usage.hpp"

#include <charconv>
#include <cstring>
#include <iostream>

Args parse_args(int argc, char** argv) {
    if (argc != 2) {
        show_usage(std::string(argv[0]));
    }

    std::size_t num_count;
    auto from_chars_result =
        std::from_chars(argv[1], argv[1] + strlen(argv[1]), num_count);

    if (from_chars_result.ec != std::errc()) {

        std::cerr << ANSI_BOLD_RED << "failed to parse <NUM> as a number"
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }

    return {.num_count = num_count};
}
