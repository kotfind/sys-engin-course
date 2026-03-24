#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace solutions {

// A Counting semaphore

// Semaphores are often used to restrict the number of threads
// than can access some (physical or logical) resource

class Semaphore {
  public:
    // Creates a Semaphore with the given number of permits
    explicit Semaphore(size_t initial_coins_count)
        : coins_count(initial_coins_count) {
    }

    // Acquires a permit from this semaphore,
    // blocking until one is available
    void Acquire() {
        std::unique_lock lock(this->mutex);
        has_coins.wait(lock, [this] { return this->coins_count > 0; });
        --this->coins_count;
    }

    // Releases a permit, returning it to the semaphore
    void Release() {
        std::lock_guard lock(this->mutex);
        ++this->coins_count;
        this->has_coins.notify_one();
    }

  private:
    std::mutex mutex;
    std::condition_variable has_coins;
    std::size_t coins_count;
};

} // namespace solutions
