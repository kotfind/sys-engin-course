#pragma once

#include "unit.hpp"

#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

class Cpu {
  public:
    Cpu(std::size_t unit_count);

    void set_data(std::size_t unit_id, std::span<const std::byte> data);

    std::vector<std::byte> move_data(std::size_t unit_id);

    int get_status_code(std::size_t unit_id);

    std::future<void> run(std::size_t unit_id);

  private:
    mutable std::mutex mutex;

    std::vector<std::unique_ptr<Unit>> units;
};
