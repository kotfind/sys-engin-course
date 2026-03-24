/// @file
///
/// @brief A "virtual cpu".

#pragma once

#include "unit.hpp"

#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>
#include <vector>

/// @brief A "virtual cpu".
///
/// Manages a set of units.
class Cpu {
  public:
    Cpu(std::size_t unit_count);

    /// @brief Returns the number of managed units.
    std::size_t get_unit_count() const;

    /// @brief Set a source code for a unit.
    void set_program_code(std::size_t unit_id, std::string_view source_code);

    /// @brief Set a data code for a unit.
    void set_data(std::size_t unit_id, std::span<const std::byte> data);

    /// @brief Moves a data from a unit.
    std::vector<std::byte> move_data(std::size_t unit_id);

    /// @brief Gets a status code from unit.
    int get_status_code(std::size_t unit_id);

    /// @brief Runs a unit.
    /// @param unit_id A unit's id.
    /// @param on_start
    ///     A function that will be called in a separate thread on unit's start.
    ///     This is required as the unit may not start immediately if unit's
    ///     busy. The argument is unit's id.
    /// @return A future, returning unit's id.
    std::future<std::size_t> run(
        std::size_t unit_id,
        std::function<void(std::size_t)> on_start = [](std::size_t) {}
    );

    /// @brief Checks if unit is running.
    bool get_is_running(std::size_t unit_id) const;

  private:
    mutable std::mutex mutex;

    /// @brief A list of managed units.
    std::vector<std::unique_ptr<Unit>> units;
};
