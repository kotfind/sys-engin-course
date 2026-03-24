#pragma once

#include <csignal>
#include <functional>
#include <vector>

/// @brief Installs @p callback as a handler for @p signals.
/// @param callback
///     A function to be called, when the signal is received.
///     Receives a signal as an argument.
/// @param signal
///     A list of signals, for which the handler should be installed.
bool install_signal_handler(
    std::function<void(int /*signal*/)> callback,
    const std::vector<int>& signals = {SIGHUP, SIGINT, SIGTERM}
);
