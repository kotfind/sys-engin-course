/// @file
///
/// @brief Some general helper classes and function.

#pragma once

#include <variant>

/// @brief A helper for `std::visit`.
template <class... Ts>
struct overloads : Ts... {
    using Ts::operator()...;
};

/// @brief Indicates, that no value were provided.
///
/// Is usually used as one of `std::variant` options
using none = std::monostate;
