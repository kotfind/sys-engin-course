#pragma once

#include <cstdint>
#include <string>
#include <string_view>

class DynamicProgram {
  public:
    ~DynamicProgram();

    static DynamicProgram* compile(const std::string& source_code);

    int run(std::uint32_t size, std::uint8_t* data) const;

  private:
    using EntryPointFn = int (*)(std::uint32_t, std::uint8_t*);

    static constexpr std::string_view entry_point_fn_mangled_name =
        "_Z10entrypointjPh";

    DynamicProgram(
        void* dynlib_handle,
        const std::string& source_code,
        EntryPointFn entry_point_fn
    );

    void* dynlib_handle;

    std::string source_code;

    EntryPointFn entry_point_fn;
};
