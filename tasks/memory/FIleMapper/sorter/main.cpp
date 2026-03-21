#include "args.hpp"
#include "log.hpp"
#include "sort.hpp"

int main(int argc, char** argv) {
    auto args = parse_args(argc, argv);

    info("Going to sort `{}`", args.file_path.string());
    info("MMap size limit = {} bytes", args.mmap_size_limit);

    uint64_file_sort(args.file_path, args.mmap_size_limit);

    info_success("Done!");
}
