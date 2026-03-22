#include "fuse_fs.hpp"

#include <memory>
#include <thread>

using namespace std::chrono_literals;

int main() {
    auto fs = std::unique_ptr<FuseFs>(FuseFs::mount("./mnt"));

    while (true) {
        std::this_thread::sleep_for(10ms);
    }
}
