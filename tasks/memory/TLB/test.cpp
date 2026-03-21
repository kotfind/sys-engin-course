#include "test.hpp"

#include "log.hpp"
#include "util.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <pthread.h>
#include <random>
#include <sched.h>
#include <vector>

std::mt19937 rnd{std::random_device{}()};

void write_on_n_pages(void* ptr, const std::vector<std::size_t>& addrs) {
    volatile char* data = (volatile char*)ptr;
    volatile char sum = 0;

    for (const std::size_t addr : addrs) {
        sum += data[addr];
    }
}

// Return average access time in nanos
double test_with_n_pages(
    std::size_t page_size, std::size_t page_count, std::size_t run_count
) {
    void* data = std::aligned_alloc(page_size, page_size * page_count);

    {
        volatile char* vdata = (volatile char*)data;
        for (size_t i = 0; i < page_size * page_count; i += page_size) {
            vdata[i] = 0xFF;
        }
    }

    std::vector<std::size_t> addrs(page_count);
    std::uniform_int_distribution<std::size_t> dist{0, page_size - 1};

    for (std::size_t page_idx = 0; page_idx < page_count; ++page_idx) {
        auto addr = page_idx * page_size + dist(rnd);
        addrs[page_idx] = addr;
    }

    std::shuffle(std::begin(addrs), std::end(addrs), rnd);

    for (std::size_t warmup = 0; warmup < 10; ++warmup) {
        write_on_n_pages(data, addrs);
    }

    auto sum_dur = with_nanos_duration([&] {
        for (std::size_t run_idx = 0; run_idx < run_count; ++run_idx) {
            write_on_n_pages(data, addrs);
        }
    });

    std::free(data);

    return sum_dur / (page_count * run_count);
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

    for (std::size_t i = 1; i < 200 + 1; i += 5) {
        auto nanos = test_with_n_pages(page_size, i, 10000000);
        info("{} -> {:.3f}ns", i, nanos);
        write_point(i, nanos);
    }
}
