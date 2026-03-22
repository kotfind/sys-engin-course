#include "fuse_file.hpp"

#include "fuse_fs.hpp"

#include <iterator>

FuseFile::~FuseFile() {
}

std::span<const std::byte> FuseFile::read() const {
    return std::span(this->data);
}

void FuseFile::set_data_unsafe(std::span<const std::byte> data) {
    this->data = std::vector(std::begin(data), std::end(data));
}

void SimpleFuseFile::write(
    std::span<const std::byte> data, std::string_view path, FuseFs* fuse_fs
) {
    fuse_fs->set_file_data(path, data);
}

SimpleFuseFile::~SimpleFuseFile() {
}
