#pragma once

#include "helpers.hpp"

#include <memory>
#include <variant>

enum class FuseEntryType : bool { Dir, File };

class FuseEntry {
  public:
    FuseEntry(FuseEntryType entry_type);

    virtual ~FuseEntry();

    FuseEntryType get_entry_type() const;

  private:
    FuseEntryType entry_type;
};

class FuseDir;
class FuseFile;

struct FuseDirEntryRef : public std::variant<none, FuseDir*, FuseFile*> {};

struct FuseDirEntryOwned
    : public std::variant<std::unique_ptr<FuseDir>, std::unique_ptr<FuseFile>> {
    FuseDirEntryRef to_ref() const;
};
