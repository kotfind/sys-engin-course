#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <mutex>

#define ANSI_BOLD "\033[1m"
#define ANSI_BOLD_RED "\033[1;31m"
#define ANSI_BOLD_GREEN "\033[1;32m"
#define ANSI_BOLD_YELLOW "\033[1;33m"
#define ANSI_CLEAR "\033[0m"

using std::chrono::steady_clock;

static const auto start_time = steady_clock::now();

template <typename LogLevel, typename AnsiColor, typename... Args>
void __log(
    LogLevel level,
    AnsiColor color,
    std::format_string<Args...>&& fmt,
    Args&&... args
) {
    const auto now = steady_clock::now();
    auto secs = std::chrono::duration<double>(now - start_time).count();

    auto msg = std::format(std::move(fmt), std::forward<Args>(args)...);

    static std::mutex mutex;
    std::lock_guard lock(mutex);
    std::cerr
        << std::format(
               "{}[{}] at {:11.5f}s: {}{}", color, level, secs, msg, ANSI_CLEAR
           )
        << std::endl;
}

template <typename... Args>
void error(std::format_string<Args...>&& fmt, Args&&... args) {
    __log("ERR", ANSI_BOLD_RED, std::move(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
void die(std::format_string<Args...>&& fmt, Args&&... args) {
    __log("ERR", ANSI_BOLD_RED, std::move(fmt), std::forward<Args>(args)...);
    exit(1);
}

template <typename... Args>
void warn(std::format_string<Args...>&& fmt, Args&&... args) {
    __log("WRN", ANSI_BOLD_YELLOW, std::move(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
void info(std::format_string<Args...>&& fmt, Args&&... args) {
    __log("INF", ANSI_BOLD, std::move(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
void success(std::format_string<Args...>&& fmt, Args&&... args) {
    __log("INF", ANSI_BOLD_GREEN, std::move(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
void trace(std::format_string<Args...>&& fmt, Args&&... args) {
    __log("TRC", "", std::move(fmt), std::forward<Args>(args)...);
}
