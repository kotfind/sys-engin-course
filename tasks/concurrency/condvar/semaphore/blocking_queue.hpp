#pragma once

#include "tagged_semaphore.hpp"

#include <queue>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
  public:
    explicit BlockingQueue(size_t capacity)
        : empty_count(capacity), filled_count(0), mutex(1) {
    }

    // Inserts the specified element into this queue,
    // waiting if necessary for space to become available.
    void Put(T value) {
        auto slot_tag = this->empty_count.Acquire();

        {
            auto lock = this->mutex.MakeGuard();
            this->buffer.push(std::move(value));
        }

        this->filled_count.Release(std::move(slot_tag));
    }

    // Retrieves and removes the head of this queue,
    // waiting if necessary until an element becomes available
    T Take() {
        auto slot_tag = this->filled_count.Acquire();

        T ans;
        {
            auto lock = this->mutex.MakeGuard();
            ans = std::move(this->buffer.front());
            this->buffer.pop();
        }

        this->empty_count.Release(std::move(slot_tag));

        return ans;
    }

  private:
    class SlotTag {};

    class MutexTag {};

    std::queue<T> buffer;

    TaggedSemaphore<SlotTag> empty_count;
    TaggedSemaphore<SlotTag> filled_count;

    TaggedSemaphore<MutexTag> mutex;
};

} // namespace solutions
