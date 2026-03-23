#include "fuse_file.hpp"

#include <algorithm>
#include <iterator>

FuseFile::~FuseFile() {
}

std::span<const std::byte> FuseFile::read() const {
    return this->get_read_data();
}

void FuseFile::write(std::span<const std::byte> data, std::size_t offset) {
    if (offset + data.size() > this->write_data.size()) {
        this->write_data.resize(offset + data.size());
    }

    std::copy(
        std::begin(data), std::end(data), std::begin(this->write_data) + offset
    );
}

void FuseFile::truncate(std::size_t size) {
    this->write_data.resize(size);
}

std::span<const std::byte> FuseFile::get_read_data() const {
    return this->read_data;
}

std::span<const std::byte> FuseFile::get_write_data() const {
    return this->write_data;
}
