#include "ansi.hpp"
#include "args.hpp"
#include "check.hpp"

#include <format>
#include <iostream>

int main(int argc, char** argv) {
    parse_args(argc, argv);

    std::cerr << ANSI_BOLD
              << "Going to check if numbers from stdin are sorted..."
              << ANSI_CLEAR << std::endl;

    if (check_numbers()) {
        std::cerr << ANSI_BOLD_GREEN << "Numbers are sorted" << ANSI_CLEAR
                  << std::endl;
    } else {
        std::cerr << ANSI_BOLD_RED << "Numbers are NOT sorted" << ANSI_CLEAR
                  << std::endl;
    }
}
