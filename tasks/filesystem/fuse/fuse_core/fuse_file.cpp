#include "fuse_file.hpp"

#include <iterator>

// -------------------- Fuse File --------------------

FuseFile::~FuseFile() {
}

std::span<const std::byte> FuseFile::read() const {
    return std::span(this->data);
}

void FuseFile::set_data_no_invalidate(std::span<const std::byte> data) {
    this->data = std::vector(std::begin(data), std::end(data));
}

// -------------------- Simple Fuse File --------------------

void SimpleFuseFile::write(std::span<const std::byte> data) {
    this->set_data_no_invalidate(data);
}

SimpleFuseFile::~SimpleFuseFile() {
}
