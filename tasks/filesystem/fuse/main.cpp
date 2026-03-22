#include "log.hpp"
#include "unit.hpp"

#include <dlfcn.h>
#include <link.h>
#include <string>
#include <vector>

int main() {
    std::string code = R"(
        #include <algorithm>
        #include <cstdint>
        #include <chrono>
        #include <thread>

        using namespace std::chrono_literals;

        int entrypoint(uint32_t size, uint8_t* ram) {
            std::sort(ram, ram + size);

            std::this_thread::sleep_for(2s);

            return 0;
        }
    )";

    auto unit = Unit(0);

    unit.set_program_code(code);
    unit.set_data({5, 4, 3, 2, 1});

    auto fut = unit.run();
    std::cout << unit.get_is_running() << std::endl;
    fut.wait();

    auto data = unit.get_data();

    for (auto x : data) {
        info("{}", x);
    }
}
