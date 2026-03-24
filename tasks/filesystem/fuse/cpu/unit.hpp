#pragma once

#include "program.hpp"

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <span>
#include <string_view>
#include <vector>

/// @brief A cpu's unit.
///
/// Stores a `data` and a `program`.
/// Can execute a `program` on a `data`.
class Unit {
  public:
    Unit(std::size_t unit_id);

    /// @brief Get this unit's id.
    std::size_t get_id() const;

    /// @brief Checks if the unit's running.
    bool get_is_running() const;

    /// @brief Set's the program's source code.
    bool set_program_code(std::string_view source_code);

    /// @brief Set's the data.
    void set_data(std::span<const std::byte> data);

    /// @brief Copies the data.
    std::vector<std::byte> copy_data() const;

    /// @brief Moves the data.
    ///
    /// Moving is done by swapping the data with an empty vector.
    /// Thus the internal data won't be in the invalid state (but will be
    /// empty).
    std::vector<std::byte> move_data();

    /// @brief Get program's status code.
    ///
    /// Calling this method never causes an undefined behavior, but it's only
    /// meaningful if the program were executed at least once.
    int get_status_code() const;

    /// @brief Runs the unit.
    ///
    /// Executes the program on the data.
    ///
    /// @param on_start
    ///     A function that will be called in a separate thread on unit's start.
    ///     This is required as the unit may not start immediately if unit's
    ///     busy.
    std::future<Unit*> run(std::function<void(Unit*)> on_start = [](Unit*) {});

  private:
    std::unique_lock<std::mutex> wait_until_finished() const;

    /// @brief A global unit mutex.
    mutable std::mutex mutex;

    /// @brief A condition variable to wait for process to stop.
    mutable std::condition_variable is_running_cv;

    /// @brief An id.
    const std::size_t id;

    /// @brief A program.
    std::unique_ptr<Program> program;

    /// @brief A data.
    std::vector<std::byte> data;

    /// @brief The program's status code.
    int status_code;

    /// @brief Is the program currently being executed?
    bool is_running;
};
