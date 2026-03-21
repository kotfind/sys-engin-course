#include "util.hpp"

#include "log.hpp"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <unistd.h>

std::size_t get_page_size() {
    auto page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        die("failed to get a page size: {}", strerror(errno));
    }

    return page_size;
}

double with_nanos_duration(const std::function<void()>& func) {
    auto start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();

    return std::chrono::duration<double, std::nano>(end - start).count();
}

void write_point(double x, double y) {
    std::cout << std::format("{:.20f} {:.20f}\n", x, y);
}
