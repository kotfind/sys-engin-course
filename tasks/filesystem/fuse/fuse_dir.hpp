#pragma once

#include "fuse_file.hpp"
#include "helpers.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

class FuseDir;

struct FuseDirEntryRef : public std::variant<FuseDir*, FuseFile*> {};

struct FuseDirEntryOwned
    : public std::variant<std::unique_ptr<FuseDir>, std::unique_ptr<FuseFile>> {
    FuseDirEntryRef to_ref() const {
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
};

class FuseDir {
  public:
    FuseDir* get_dir(std::string_view path) const;

    FuseFile* get_file(std::string_view path) const;

    bool add_dir(std::string_view path, std::unique_ptr<FuseDir> dir);

    bool add_file(std::string_view path, std::unique_ptr<FuseFile> file);

  private:
    std::unordered_map<std::string, std::unique_ptr<FuseDir>> dirs;

    std::unordered_map<std::string, std::unique_ptr<FuseFile>> files;

    FuseDir* get_dir_here(std::string_view name) const;

    FuseFile* get_file_here(std::string_view name) const;

    bool add_dir_here(std::string_view name, std::unique_ptr<FuseDir> dir);

    bool add_file_here(std::string_view name, std::unique_ptr<FuseFile> file);
};
