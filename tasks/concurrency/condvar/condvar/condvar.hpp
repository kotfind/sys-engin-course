#pragma once

#include <atomic>
#include <cstdint>

#include "BasicLockable.hpp"

namespace stdlike {

class CondVar {
  public:
    template <BasicLockable Mutex>
    void Wait(Mutex& mutex) {
        auto old = this->inner.load(std::memory_order_relaxed);

        mutex.unlock();
        inner.wait(old, std::memory_order_relaxed);
        mutex.lock();
    }

    void NotifyOne() {
        this->inner.fetch_add(1, std::memory_order_acq_rel);
        this->inner.notify_one();
    }

    void NotifyAll() {
        this->inner.fetch_add(1, std::memory_order_acq_rel);
        this->inner.notify_all();
    }

  private:
    std::atomic<uint32_t> inner;
};

} // namespace stdlike
