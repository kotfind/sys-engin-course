#include "signal_handler.hpp"
#include "log.hpp"

#include <cstring>
#include <functional>

static std::function<void(int /*signal*/)> callback;

static void signal_handler(int signal) {
    callback(signal);
}

bool install_signal_handler(
    std::function<void(int /*signal*/)> callback_,
    const std::vector<int>& signals
) {
    callback = callback_;

    struct sigaction sa;

    std::memset(&sa, 0, sizeof(sa));

    sa.sa_handler = signal_handler;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (auto signal : signals) {
        if (sigaction(signal, &sa, nullptr) != 0) {
            error("Failed to install signal handler for signal={}", signal);
            return false;
        }
    }

    success("Installed signal handler");
    return true;
}
