/// @file
///
/// @brief A dynamic program.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

/// @brief A dynamic program.
///
/// This class works by executing a `clang++` executable on a piece of code to
/// compile it into a shared library, which is later loaded and called.
///
/// The piece of code is required to a function with the following signature:
/// ```cpp
/// int entrypoint(std::uint32_t, std::uint8_t*);
/// ```
class Program {
  public:
    ~Program();

    /// @brief Tries to compile a source_code and load it as a library.
    /// @return A new instance or nullptr of fail.
    static Program* compile(std::string_view source_code);

    /// @brief Create a "dummy" program.
    ///
    /// Dummy program does nothing, when called.
    static Program* dummy();

    /// @brief Runs a program.
    /// @param data Data to be passed to `entrypoint` function.
    /// @return Return value of `entrypoint` function.
    int run(std::span<std::byte> data) const;

  private:
    /// @brief A type of `entrypoint` function pointer.
    using EntryPointFn = int (*)(std::uint32_t, std::uint8_t*);

    /// @brief Extra flags to pass to `clang++` compiler.
    static const std::string_view clang_compile_extra_flags;

    /// @brief A mangled name of entrypoint function.
    ///
    /// It doesn't seem too stable, but I don't know no better solutions.
    static const std::string_view entry_point_fn_mangled_name;

    Program(
        void* dynlib_handle,
        std::string_view source_code,
        EntryPointFn entry_point_fn
    );

    /// @brief A handle to the loaded dynamic library.
    void* dynlib_handle;

    /// @brief A source code of the program.
    std::string source_code;

    /// @brief A pointer to `entrypoint` function.
    EntryPointFn entry_point_fn;
};
