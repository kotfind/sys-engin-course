#include "fuse_dir.hpp"
#include "log.hpp"

#include <cassert>
#include <string_view>
#include <utility>

std::pair<std::string_view, std::string_view> split_path_head(
    std::string_view path
) {
    auto sep_pos = path.find_first_of('/');

    assert(sep_pos != std::string_view::npos);

    auto head = path.substr(0, sep_pos);
    auto tail = path.substr(sep_pos + 1);

    return {head, tail};
}

FuseDir* FuseDir::get_dir(std::string_view path) const {
    if (path.find("/") == std::string_view::npos) {
        return this->get_dir_here(path);
    }

    auto [head, tail] = split_path_head(path);

    auto next_dir = get_dir_here(head);
    if (next_dir == nullptr) {
        return nullptr;
    }

    return next_dir->get_dir(tail);
}

FuseFile* FuseDir::get_file(std::string_view path) const {
    if (path.find("/") == std::string_view::npos) {
        return this->get_file_here(path);
    }

    auto [head, tail] = split_path_head(path);

    auto next_dir = get_dir_here(head);
    if (next_dir == nullptr) {
        return nullptr;
    }

    return next_dir->get_file(tail);
}

bool FuseDir::add_dir(std::string_view path, std::unique_ptr<FuseDir> dir) {
    if (path.find("/") == std::string_view::npos) {
        return this->get_dir_here(path);
    }

    auto [head, tail] = split_path_head(path);

    auto next_dir = get_dir_here(head);
    if (next_dir == nullptr) {
        return false;
    }

    return next_dir->add_dir(tail, std::move(dir));
}

bool FuseDir::add_file(std::string_view path, std::unique_ptr<FuseFile> file) {
    if (path.find("/") == std::string_view::npos) {
        return this->get_dir_here(path);
    }

    auto [head, tail] = split_path_head(path);

    auto next_dir = get_dir_here(head);
    if (next_dir == nullptr) {
        return false;
    }

    return next_dir->add_file(tail, std::move(file));
}

FuseDir* FuseDir::get_dir_here(std::string_view name_view) const {
    // NOTE: I know this allocates and thus is pretty slow.
    // But std::unordered_map<std::string, T> does not support
    // std::string_view lookup and I don't quite want to
    // define custom hasher and comparator. So, allocation it is.
    const std::string name{name_view};

    assert(name.find("/") == std::string_view::npos);

    auto it = this->dirs.find(name);
    if (it != this->dirs.end()) {
        return it->second.get();
    } else {
        error("Dir `{}` does not exist", name);
        return nullptr;
    }
}

FuseFile* FuseDir::get_file_here(std::string_view name_view) const {
    const std::string name{name_view};

    assert(name.find("/") == std::string_view::npos);

    auto it = this->files.find(name);
    if (it != this->files.end()) {
        return it->second.get();
    } else {
        error("File `{}` does not exist", name);
        return nullptr;
    }
}

bool FuseDir::add_dir_here(
    std::string_view name_view, std::unique_ptr<FuseDir> dir
) {
    const std::string name{name_view};

    assert(name.find("/") == std::string_view::npos);

    if (this->dirs.contains(name) || this->files.contains(name)) {
        error("failed to create  `{}`: path already exists", name);
        return false;
    }

    this->dirs[name] = std::move(dir);
    return true;
}

bool FuseDir::add_file_here(
    std::string_view name_view, std::unique_ptr<FuseFile> file
) {
    const std::string name{name_view};

    assert(name.find("/") == std::string_view::npos);

    if (this->dirs.contains(name) || this->files.contains(name)) {
        error("failed to create `{}`: path already exists", name);
        return false;
    }

    this->files[name] = std::move(file);
    return true;
}
