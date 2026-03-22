#pragma once

#include "program.hpp"

#include <condition_variable>
#include <cstdint>
#include <future>
#include <mutex>
#include <vector>

class Unit {
  public:
    Unit(std::size_t unit_id);

    std::size_t get_id() const;

    bool get_is_running() const;

    bool set_program_code(const std::string& source_code);

    void set_data(const std::vector<std::uint8_t>& data);

    std::vector<std::uint8_t> get_data() const;

    int get_status_code() const;

    std::future<Unit*> run();

  private:
    std::unique_lock<std::mutex> wait_until_finished() const;

    mutable std::mutex mutex;

    mutable std::condition_variable is_running_cv;

    const std::size_t id;

    std::unique_ptr<Program> program;

    std::vector<std::uint8_t> data;

    int status_code;

    bool is_running;
};
