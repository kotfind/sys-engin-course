#include "fuse_dir.hpp"
#include "fuse_fs.hpp"

#include <memory>
#include <thread>

using namespace std::chrono_literals;

int main() {
    auto root_dir = std::make_unique<FuseDir>();
    auto fs =
        std::unique_ptr<FuseFs>(FuseFs::mount("./mnt", std::move(root_dir)));

    while (true) {
        std::this_thread::sleep_for(10ms);
    }
}
