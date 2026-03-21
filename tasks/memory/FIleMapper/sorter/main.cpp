#include "ansi.hpp"
#include "args.hpp"
#include "log.hpp"
#include "sort.hpp"
#include "uint64_file.hpp"

#include <fcntl.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <sys/mman.h>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    auto args = parse_args(argc, argv);

    info("Going to sort `{}`", args.file_path.string());
    info("MMap memory limit = {} bytes", args.memory_limit);

    auto file = UInt64File(args.file_path, args.memory_limit);
    uint64_file_sort(file);

    info_success("Done!");
}
