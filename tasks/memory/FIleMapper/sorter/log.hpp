#pragma once

#include "ansi.hpp"

#include <chrono>
#include <format>
#include <iostream>

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
void info_success(std::format_string<Args...>&& fmt, Args&&... args) {
    __log("INF", ANSI_BOLD_GREEN, std::move(fmt), std::forward<Args>(args)...);
}
