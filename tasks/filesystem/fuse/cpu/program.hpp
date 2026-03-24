#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

class Program {
  public:
    ~Program();

    static Program* compile(std::string_view source_code);

    static Program* dummy();

    int run(std::span<std::byte>) const;

  private:
    using EntryPointFn = int (*)(std::uint32_t, std::uint8_t*);

    static const std::string_view clang_compile_extra_flags;

    static const std::string_view entry_point_fn_mangled_name;

    Program(
        void* dynlib_handle,
        std::string_view source_code,
        EntryPointFn entry_point_fn
    );

    void* dynlib_handle;

    std::string source_code;

    EntryPointFn entry_point_fn;
};
