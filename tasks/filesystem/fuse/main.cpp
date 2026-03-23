#include "fuse_cpu.hpp"
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

    auto cpu = std::unique_ptr<FuseCpu>(FuseCpu::create("./mnt", 5));

    std::mutex mutex;
    std::unique_lock lock{mutex};
    is_running_cv.wait(lock, [&]() { return !is_running; });
}
