#include "ansi.hpp"
#include "args.hpp"
#include "uint64_file.hpp"

#include <fcntl.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <sys/mman.h>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    auto args = parse_args(argc, argv);

    std::cerr << ANSI_BOLD
              << std::format(
                     "Going to sort `{}` (memory_limit = {} bytes)",
                     args.file_path.string(),
                     args.memory_limit
                 )
              << ANSI_CLEAR << std::endl;

    auto file = UInt64File(args.file_path, args.memory_limit);

    for (std::size_t item_idx = 0; item_idx < file.get_items_cont();
         ++item_idx) {
        std::cout << file[item_idx] << "\n";
    }
}
