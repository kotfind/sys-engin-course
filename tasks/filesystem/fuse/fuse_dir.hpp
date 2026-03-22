#pragma once

#include "fuse_entry.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

class FuseDir : FuseEntry {
  public:
    FuseDir();

    FuseDirEntryRef get_entry(std::string_view path);

    bool add_entry(std::string_view path, FuseDirEntryOwned entry);

    const std::unordered_map<std::string, FuseDirEntryOwned>&
    view_entries() const;

  private:
    std::unordered_map<std::string, FuseDirEntryOwned> entries;

    FuseDirEntryRef get_entry_here(std::string_view name);

    bool add_entry_here(std::string_view name, FuseDirEntryOwned entry);
};
