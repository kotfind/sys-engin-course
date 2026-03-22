#pragma once

#include <span>
#include <string_view>

class FuseFile {
  public:
    FuseFile() {
        // TODO: REMOVE ME
    }

    std::span<const std::byte> get_data() {
        // TODO: REMOVE ME
        static constexpr std::string_view content = "Hello, world!";
        return std::as_bytes(std::span(content));
    }

    // TODO
};
