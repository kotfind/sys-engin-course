#include "fuse_dir.hpp"
#include "fuse_file.hpp"
#include "fuse_fs.hpp"

#include <memory>
#include <thread>

using namespace std::chrono_literals;

int main() {
    auto root_dir = std::make_unique<FuseDir>();

    // .
    // |
    // |- a
    // |  \- b
    // |     \- 1.txt
    // |- c
    // \- d
    //    \- 2.txt

    root_dir->add_entry("/a", {std::make_unique<FuseDir>()});
    root_dir->add_entry("a/b", {std::make_unique<FuseDir>()});
    root_dir->add_entry("/a/b/1.txt", {std::make_unique<FuseFile>()});

    root_dir->add_entry("c", {std::make_unique<FuseDir>()});
    root_dir->add_entry("d", {std::make_unique<FuseDir>()});
    root_dir->add_entry("d/2.txt", {std::make_unique<FuseFile>()});

    auto fs =
        std::unique_ptr<FuseFs>(FuseFs::mount("./mnt", std::move(root_dir)));

    if (fs == nullptr) {
        return 1;
    }

    while (true) {
        std::this_thread::sleep_for(10ms);
    }
}
