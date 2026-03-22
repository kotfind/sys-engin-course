#include "dynamic_program.hpp"
#include "log.hpp"

#include <dlfcn.h>
#include <link.h>
#include <memory>
#include <string>
#include <vector>

int main() {
    std::string code = R"(
        #include <algorithm>
        #include <cstdint>

        int entrypoint(uint32_t size, uint8_t* ram) {
            std::sort(ram, ram + size);
            return 0;
        }
    )";

    auto prog = std::unique_ptr<DynamicProgram>(DynamicProgram::compile(code));
    if (prog == nullptr) {
        exit(1);
    }

    std::vector<uint8_t> data = {5, 4, 3, 2, 1};
    prog->run(data.size(), data.data());

    for (auto x : data) {
        info("{}", x);
    }
}
