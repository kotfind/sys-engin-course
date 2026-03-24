#include "args.hpp"
#include "run.hpp"

int main(int argc, char** argv) {
    return run(parse_args(argc, argv));
}
