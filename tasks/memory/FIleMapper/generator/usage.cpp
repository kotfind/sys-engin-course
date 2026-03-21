#include "usage.hpp"

#include "ansi.hpp"

#include <format>
#include <iostream>
#include <string>

void show_usage(const std::string argv0, bool is_error) {
    if (is_error) {
        std::cerr << ANSI_BOLD_RED << "Failed to parse command line arguments\n"
                  << ANSI_CLEAR << std::endl;
    }

    // clang-format off
    std::cerr << std::format(
        ANSI_BOLD_F("Usage:\n")
        "{} <NUM>\n"
        "\n"
        ANSI_BOLD_F("Description:\n")
        "Generates an output with sequence of unsigned 64-bit integers\n"
        "\n"
        ANSI_BOLD_F("Options\n")
        "<NUM>    Number of integers to generate\n",
        argv0)
        << std::endl;
    // clang-format on

    exit(is_error);
}
