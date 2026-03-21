#include "args.hpp"

#include "ansi.hpp"
#include "log.hpp"

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
        die("file `{}` does not exists", file_path.string());
    }

    // ---------- MMap Size Limit ----------

    std::size_t mmap_size_limit;
    if (argc == 3) {
        auto from_chars_result = std::from_chars(
            argv[2], argv[2] + strlen(argv[2]), mmap_size_limit
        );

        if (from_chars_result.ec != std::errc()) {
            die("failed to parse [MMAP_SIZE_LIMIT] as a number");
        }

        mmap_size_limit *= 1024 * 1024; // MB -> B
    } else {
        mmap_size_limit = fs::file_size(file_path) / 10;
    }

    return {.file_path = file_path, .mmap_size_limit = mmap_size_limit};
}

void show_usage(const std::string argv0) {
    error("Failed to parse command line arguments\n");

    // clang-format off
    std::cerr << std::format(
        ANSI_BOLD_F("Usage:\n")
        "{} <FILE_PATH> [MMAP_SIZE_LIMIT]\n"
        "\n"
        ANSI_BOLD_F("Description:\n")
        "Sorts FILE_PATH, using no more than MMAP_SIZE_LIMIT MB of mmap-binded memory.\n"
        "\n"
        ANSI_BOLD_F("Options\n")
        "<FILE_PATH>  Path to a file to be sorted.\n"
        "             File should contain binary representation of 64-bit unsigned integers.\n"
        "\n"
        "[MMAP_SIZE_LIMIT]\n"
        "             Maximum allowed amout of mmap-binded memory in MB.\n"
        "             Default: size of file <FILE_PATH> divided by 10.",
        argv0)
        << std::endl;

    exit(1);
}
