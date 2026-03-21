#include "test.hpp"

#include "log.hpp"
#include "util.hpp"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <pthread.h>
#include <sched.h>
#include <vector>

volatile char black_box;

void write_on_n_pages(std::vector<char>& data, std::size_t page_size) {
    for (std::size_t i = 0; i < data.size(); i += page_size) {
        black_box = data[i];
    }
}

// Return average access time in nanos
double test_with_n_pages(
    std::size_t page_size, std::size_t page_count, std::size_t run_count
) {
    std::vector<char> data(page_size * page_count, 0);

    // do some runs to put pages to TLB
    for (std::size_t run_idx = 0; run_idx < 5; ++run_idx) {
        write_on_n_pages(data, page_size);
    }

    auto start = std::chrono::steady_clock::now();
    for (std::size_t run_idx = 0; run_idx < run_count; ++run_idx) {
        write_on_n_pages(data, page_size);
    }
    auto end = std::chrono::steady_clock::now();

    auto sum_dur =
        std::chrono::duration<double, std::nano>(end - start).count();

    return sum_dur / page_count / run_count;
}

void pin_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();

    auto err =
        pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);

    if (err != 0) {
        die("failed to pin to core: {}", strerror(err));
    }
}

void run_tests() {
    auto page_size = get_page_size();

    pin_to_core(2);

    for (std::size_t i = 1; i < 1000 + 2; i += 1) {
        auto nanos = test_with_n_pages(page_size, i, 1000);
        info("{} -> {:.3f}ns", i, nanos);
        write_point(i, nanos);
    }
}
