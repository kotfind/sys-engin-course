#include "args.hpp"

#include "ansi.hpp"

#include <charconv>
#include <cstring>
#include <format>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

Args parse_args(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        show_usage(std::string(argv[0]));
    }

    // ---------- File Path ----------

    fs::path file_path{argv[1]};
    file_path = fs::absolute(file_path);

    if (!fs::exists(file_path)) {
        std::cerr << ANSI_BOLD_RED
                  << std::format("file `{}` does not exists",
                                 file_path.string())
                  << ANSI_CLEAR << std::endl;
        exit(1);
    }

    // ---------- Memory Limit ----------

    std::size_t memory_limit;
    if (argc == 3) {
        auto from_chars_result =
            std::from_chars(argv[2], argv[2] + strlen(argv[2]), memory_limit);

        if (from_chars_result.ec != std::errc()) {
            std::cerr << ANSI_BOLD_RED
                      << "failed to parse <MEMORY_LIMIT> as a number"
                      << ANSI_CLEAR << std::endl;
            exit(1);
        }

        memory_limit *= 1024 * 1024; // MB -> B
    } else {
        memory_limit = fs::file_size(file_path) / 10;
    }

    return {.file_path = file_path, .memory_limit = memory_limit};
}

void show_usage(const std::string argv0) {
    std::cerr << ANSI_BOLD_RED << "Failed to parse command line arguments\n"
              << ANSI_CLEAR << std::endl;

    // clang-format off
    std::cerr << std::format(
        ANSI_BOLD_F("Usage:\n")
        "{} <FILE_PATH> [MEMORY_LIMIT]\n"
        "\n"
        ANSI_BOLD_F("Description:\n")
        "Sorts FILE_PATH, using no more than MEMORY_LIMIT memory.\n"
        "\n"
        ANSI_BOLD_F("Options\n")
        "<FILE_PATH>    Path to a file to be sorted.\n"
        "               File should contain binary representation of 64-bit unsigned integers.\n"
        "\n"
        "[MEMORY_LIMIT] Maximum allowed amout of memory in MB.\n"
        "               Default: size of file <FILE_PATH> divided by 10.",
        argv0)
        << std::endl;
    // clang-format on

    exit(1);
}
