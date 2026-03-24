#pragma once

#include "fuse_file.hpp"
#include "helpers.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

class FuseDir;

/// @brief Holds a non-owned reference to a directory's entry or none.
struct FuseDirEntryRef : public std::variant<none, FuseDir*, FuseFile*> {};

/// @brief Holds an owned value of directory's entry (NOT none).
struct FuseDirEntryOwned
    : public std::variant<std::unique_ptr<FuseDir>, std::unique_ptr<FuseFile>> {
    FuseDirEntryRef to_ref() const;
};

/// @brief A directory.
class FuseDir {
  public:
    /// @brief Tries to get a directory.
    /// @return A pointer to the directory or nullptr.
    FuseDir* get_dir(std::string_view path);

    /// @brief Tries to get a file.
    /// @return A pointer to the file or nullptr.
    FuseFile* get_file(std::string_view path);

    /// @brief Tries to add a directory.
    /// @return Did the operation succeed?
    bool add_dir(std::string_view path, std::unique_ptr<FuseDir> dir);

    /// @brief Tries to add a file.
    /// @return Did the operation succeed?
    bool add_file(std::string_view path, std::unique_ptr<FuseFile> file);

    /// @brief Tries to get an entry.
    FuseDirEntryRef get_entry(std::string_view path);

    /// @brief Tries to add an entry.
    /// @return Did the operation succeed?
    bool add_entry(std::string_view path, FuseDirEntryOwned entry);

    /// @brief Get a list of entries.
    const std::unordered_map<std::string, FuseDirEntryOwned>&
    view_entries() const;

  private:
    std::unordered_map<std::string, FuseDirEntryOwned> entries;

    /// @brief Get an entry from the current directory.
    ///
    /// Doesn't allow fetching from subdirectories.
    FuseDirEntryRef get_entry_here(std::string_view name);

    /// @brief Adds an entry to the current directory.
    ///
    /// Doesn't allow adding from subdirectories.
    bool add_entry_here(std::string_view name, FuseDirEntryOwned entry);
};
