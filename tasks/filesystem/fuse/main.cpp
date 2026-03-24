#include "args.hpp"
#include "fuse_cpu.hpp"
#include "log.hpp"
#include "signal_handler.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>

using namespace std::chrono_literals;

int run(const Args& args) {
    bool is_running = true;
    std::condition_variable is_running_cv;

    if (!install_signal_handler([&](int signal) {
            is_running = false;
            is_running_cv.notify_all();
            info("Got signal={}. Exiting...", signal);
        })) {
        return 1;
    }

    auto cpu = std::unique_ptr<FuseCpu>(
        FuseCpu::create(args.mount_point, args.unit_count)
    );
    if (cpu == nullptr) {
        return 1;
    }

    std::mutex mutex;
    std::unique_lock lock{mutex};
    is_running_cv.wait(lock, [&]() { return !is_running; });

    return 0;
}

int main(int argc, char** argv) {
    auto args = parse_args(argc, argv);

    return run(args);
}
