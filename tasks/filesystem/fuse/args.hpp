#pragma once

#include <cstddef>
#include <filesystem>

/// @brief Command line arguments.
///
/// Is parsed from `argc` and `argv` by @ref parse_args.
struct Args {
    /// @brief A number of cpu units to create.
    std::size_t unit_count;

    /// @brief Where to mount a filesystem.
    ///
    /// Both relative and absolute paths are allowed.
    std::filesystem::path mount_point;
};

/// @brief Parses @ref Args from `argc` and `argv`.
Args parse_args(int argc, char** argv);
