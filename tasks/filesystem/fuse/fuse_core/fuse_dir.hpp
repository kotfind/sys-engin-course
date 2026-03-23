#pragma once

#include "fuse_file.hpp"
#include "helpers.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

class FuseDir;

struct FuseDirEntryRef : public std::variant<none, FuseDir*, FuseFile*> {};

struct FuseDirEntryOwned
    : public std::variant<std::unique_ptr<FuseDir>, std::unique_ptr<FuseFile>> {
    FuseDirEntryRef to_ref() const;
};

class FuseDir {
  public:
    FuseDir* get_dir(std::string_view path);

    FuseFile* get_file(std::string_view path);

    bool add_dir(std::string_view path, std::unique_ptr<FuseDir> dir);

    bool add_file(std::string_view path, std::unique_ptr<FuseFile> file);

    FuseDirEntryRef get_entry(std::string_view path);

    bool add_entry(std::string_view path, FuseDirEntryOwned entry);

    const std::unordered_map<std::string, FuseDirEntryOwned>&
    view_entries() const;

  private:
    std::unordered_map<std::string, FuseDirEntryOwned> entries;

    FuseDirEntryRef get_entry_here(std::string_view name);

    bool add_entry_here(std::string_view name, FuseDirEntryOwned entry);
};
