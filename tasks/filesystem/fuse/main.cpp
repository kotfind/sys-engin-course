#include "args.hpp"
#include "run.hpp"

/// @brief An entry point to the program.
///
/// Just calls @ref parse_args and @ref run functions.
int main(int argc, char** argv) {
    return run(parse_args(argc, argv));
}
