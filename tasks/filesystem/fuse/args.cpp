#include "args.hpp"
#include "log.hpp"
#include <charconv>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <format>
#include <string_view>
#include <system_error>

static constexpr std::string_view default_mount_point = "./mnt";

static void print_usage(
    std::string_view argv0, std::string_view maybe_error = ""
) {
    static constexpr std::string_view msg = R"(
{1}Description:{0}
Creates a "virtual cpu" with FUSE filesystem interface.

{1}Usage:{0}
{1}{2}{0} <UNIT_COUNT> [MOUNT_POINT]

{1}Options:{0}
<UNIT_COUNT>    Number of processor units.

[MOUNT_POINT]   Where to mount a cpu filesystem.
                Default: {3}
)";

    if (!maybe_error.empty()) {
        error("Failed to parse arguments");
    }

    const auto formatted =
        std::format(msg, ANSI_CLEAR, ANSI_BOLD, argv0, default_mount_point);

    std::cout << formatted << std::endl;

    exit(maybe_error.empty() ? 0 : 1);
}

Args parse_args(int argc, char** argv) {
    if (argc >= 2) {
        auto argv1 = std::string_view(argv[1]);
        if (argv1 == "-h" || argv1 == "--help") {
            print_usage(argv[0]);
        }
    }

    if (argc != 2 && argc != 3) {
        print_usage(argv[0], "wrong number arguments");
    }

    std::size_t unit_count;
    auto from_chars_res =
        std::from_chars(argv[1], argv[1] + strlen(argv[1]), unit_count);
    if (from_chars_res.ec != std::errc()) {
        print_usage(argv[0], "failed to parse <UNIT_COUNT> as int");
    }

    auto mount_point =
        argc >= 3 ? std::filesystem::path(argv[2]) : default_mount_point;

    return Args{.unit_count = unit_count, .mount_point = mount_point};
}
