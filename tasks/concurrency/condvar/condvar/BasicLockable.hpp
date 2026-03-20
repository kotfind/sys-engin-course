#pragma once

namespace stdlike {

// https://en.cppreference.com/w/cpp/named_req/BasicLockable
template <typename Mutex>
concept BasicLockable = requires(Mutex& m) {
    m.lock();
    m.unlock();
};

} // namespace stdlike
