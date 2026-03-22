#pragma once

#include <csignal>
#include <functional>
#include <vector>

bool install_signal_handler(
    std::function<void(int /*signal*/)> callback,
    const std::vector<int>& signals = {SIGHUP, SIGINT, SIGTERM}
);
