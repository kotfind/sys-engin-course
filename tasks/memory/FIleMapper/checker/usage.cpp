#include "usage.hpp"

#include "ansi.hpp"

#include <format>
#include <iostream>
#include <string>

void show_usage(const std::string argv0) {
    std::cerr << ANSI_BOLD_RED << "Failed to parse command line arguments\n"
              << ANSI_CLEAR << std::endl;

    // clang-format off
    std::cerr << std::format(
        ANSI_BOLD_F("Usage:\n")
        "{}\n"
        "\n"
        ANSI_BOLD_F("Description:\n")
        "Checks if unsigned 64-bit integers from stdout are sorted\n",
        argv0)
        << std::endl;
    // clang-format on

    exit(1);
}
