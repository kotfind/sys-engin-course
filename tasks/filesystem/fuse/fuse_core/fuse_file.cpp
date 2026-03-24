#include "fuse_file.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

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

void FuseFile::set_read_data(std::span<const std::byte> data) {
    this->read_data.assign(std::begin(data), std::end(data));
}

std::span<const std::byte> FuseFile::get_read_data() const {
    return this->read_data;
}

std::vector<std::byte> FuseFile::move_write_data() {
    std::vector<std::byte> ans;
    ans.swap(this->write_data);
    return ans;
}

std::span<const std::byte> FuseFile::get_write_data() const {
    return this->write_data;
}
