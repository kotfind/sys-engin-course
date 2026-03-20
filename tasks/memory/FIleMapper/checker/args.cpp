#include "args.hpp"

#include "ansi.hpp"
#include "usage.hpp"

#include <string>

void parse_args(int argc, char** argv) {
    if (argc != 1) {
        show_usage(std::string(argv[0]));
    }
}
