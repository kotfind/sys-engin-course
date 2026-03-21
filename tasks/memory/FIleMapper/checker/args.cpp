#include "args.hpp"

#include "ansi.hpp"
#include "usage.hpp"

#include <cstring>
#include <string>

void parse_args(int argc, char** argv) {
    if (argc >= 2 &&
        (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        show_usage(std::string(argv[0]), false);
    }

    if (argc >= 2) {
        show_usage(std::string(argv[0]), true);
    }
}
