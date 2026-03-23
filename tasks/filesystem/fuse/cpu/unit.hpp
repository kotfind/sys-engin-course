#pragma once

#include "program.hpp"

#include <condition_variable>
#include <cstddef>
#include <future>
#include <mutex>
#include <span>
#include <string_view>
#include <vector>

class Unit {
  public:
    Unit(std::size_t unit_id);

    std::size_t get_id() const;

    bool get_is_running() const;

    bool set_program_code(std::string_view source_code);

    void set_data(std::span<const std::byte> data);

    std::vector<std::byte> copy_data() const;

    std::vector<std::byte> move_data();

    int get_status_code() const;

    std::future<Unit*> run();

  private:
    std::unique_lock<std::mutex> wait_until_finished() const;

    mutable std::mutex mutex;

    mutable std::condition_variable is_running_cv;

    const std::size_t id;

    std::unique_ptr<Program> program;

    std::vector<std::byte> data;

    int status_code;

    bool is_running;
};
