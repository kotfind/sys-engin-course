#include "fuse_dir.hpp"
#include "fuse_file.hpp"
#include "fuse_fs.hpp"
#include "log.hpp"
#include "signal_handler.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>

using namespace std::chrono_literals;

int main() {
    bool is_running = true;
    std::condition_variable is_running_cv;

    if (!install_signal_handler([&](int signal) {
            is_running = false;
            is_running_cv.notify_all();
            info("Got signal={}. Exiting...", signal);
        })) {
        return 1;
    }

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

    std::mutex mutex;
    std::unique_lock lock{mutex};
    is_running_cv.wait(lock, [&]() { return !is_running; });
}
