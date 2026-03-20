#pragma once

#include <cstdint>

struct Args {
    std::size_t num_count;
};

Args parse_args(int argc, char** argv);
