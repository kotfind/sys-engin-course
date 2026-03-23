#include "fuse_dir.hpp"
#include "fuse_file.hpp"
#include "helpers.hpp"
#include "log.hpp"

#include <cassert>
#include <string_view>
#include <utility>
#include <variant>

static std::string_view prepare_path(std::string_view path) {
    return path.starts_with("/") ? path.substr(1) : path;
}

std::pair<std::string_view, std::string_view> split_path_head(
    std::string_view path
) {
    auto sep_pos = path.find_first_of('/');

    assert(sep_pos != std::string_view::npos);

    auto head = path.substr(0, sep_pos);
    auto tail = path.substr(sep_pos + 1);

    return {head, tail};
}

FuseDirEntryRef FuseDirEntryOwned::to_ref() const {
    return std::visit(
        overloads{
            [](const std::unique_ptr<FuseDir>& dir) {
                return FuseDirEntryRef(dir.get());
            },
            [](const std::unique_ptr<FuseFile>& file) {
                return FuseDirEntryRef(file.get());
            }
        },
        *this
    );
}

FuseDir* FuseDir::get_dir(std::string_view path) {
    auto entry = this->get_entry(path);
    return std::visit(
        overloads{
            [](FuseDir* dir) { return dir; },
            [](FuseFile*) {
                error("Failed to get dir: this is a regular file");
                return (FuseDir*)nullptr;
            },
            [](none) {
                error("Faield to get dir: path does not exist");
                return (FuseDir*)nullptr;
            },
        },
        entry
    );
}

FuseFile* FuseDir::get_file(std::string_view path) {
    auto entry = this->get_entry(path);
    return std::visit(
        overloads{
            [](FuseFile* file) { return file; },
            [](FuseDir*) {
                error("Failed to get file: this is a dir");
                return (FuseFile*)nullptr;
            },
            [](none) {
                error("Failed to get file: path does not exist");
                return (FuseFile*)nullptr;
            },
        },
        entry
    );
}

bool FuseDir::add_dir(std::string_view path, std::unique_ptr<FuseDir> dir) {
    return this->add_entry(path, {std::move(dir)});
}

bool FuseDir::add_file(std::string_view path, std::unique_ptr<FuseFile> file) {
    return this->add_entry(path, {std::move(file)});
}

FuseDirEntryRef FuseDir::get_entry(std::string_view path) {
    path = prepare_path(path);

    if (path.empty()) {
        return {this};
    }

    if (path.find("/") == std::string_view::npos) {
        return this->get_entry_here(path);
    }

    auto [head, tail] = split_path_head(path);

    auto next_entry = this->get_entry_here(head);
    auto* next_dir = std::visit(
        overloads{
            [](FuseDir* dir) { return dir; },
            [head](FuseFile*) {
                error("cannot enter `{}` it's not a directory", head);
                return (FuseDir*)nullptr;
            },
            [head](none) {
                error("cannot enter `{}`: it doesn't exist", head);
                return (FuseDir*)nullptr;
            },
        },
        next_entry
    );
    if (next_dir == nullptr) {
        return {};
    }

    return next_dir->get_entry(tail);
}

// TODO: remove code repetition: this method is hardly different from
// `get_entry`
bool FuseDir::add_entry(std::string_view path, FuseDirEntryOwned entry) {
    path = prepare_path(path);

    assert(!path.empty());

    if (path.find("/") == std::string_view::npos) {
        return this->add_entry_here(path, std::move(entry));
    }

    auto [head, tail] = split_path_head(path);

    auto next_entry = this->get_entry_here(head);
    auto* next_dir = std::visit(
        overloads{
            [](FuseDir* dir) { return dir; },
            [head](FuseFile*) {
                error("cannot enter `{}` it's not a directory", head);
                return (FuseDir*)nullptr;
            },
            [head](none) {
                error("cannot enter `{}`: it doesn't exist", head);
                return (FuseDir*)nullptr;
            },
        },
        next_entry
    );
    if (next_dir == nullptr) {
        return false;
    }

    return next_dir->add_entry(tail, std::move(entry));
}

const std::unordered_map<std::string, FuseDirEntryOwned>& FuseDir::
    view_entries() const {
    return this->entries;
}

FuseDirEntryRef FuseDir::get_entry_here(std::string_view name_view) {
    // NOTE: I know this allocates and thus is pretty slow.
    // But std::unordered_map<std::string, T> does not support
    // std::string_view lookup and I don't quite want to
    // define custom hasher and comparator. So, allocation it is.
    const std::string name{name_view};

    assert(name.find("/") == std::string_view::npos);

    auto it = this->entries.find(name);
    if (it != this->entries.end()) {
        return it->second.to_ref();
    } else {
        error("Path `{}` does not exist", name);
        return {};
    }
}

bool FuseDir::add_entry_here(
    std::string_view name_view, FuseDirEntryOwned entry
) {
    const std::string name{name_view};

    assert(name.find("/") == std::string_view::npos);

    if (this->entries.contains(name)) {
        error("failed to create `{}`: path already exists", name);
        return false;
    }

    this->entries.emplace(name, std::move(entry));
    return true;
}
