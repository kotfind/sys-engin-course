#include "ansi.hpp"
#include "args.hpp"

#include <format>
#include <iostream>

int main(int argc, char** argv) {
    auto args = parse_args(argc, argv);

    std::cerr << ANSI_BOLD
              << std::format("Going to sort `{}` (memory_limit = {} bytes)",
                             args.file_path.string(), args.memory_limit)
              << ANSI_CLEAR << std::endl;
}
