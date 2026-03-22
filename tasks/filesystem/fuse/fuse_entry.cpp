#include "fuse_entry.hpp"

FuseEntry::FuseEntry(FuseEntryType entry_type) : entry_type(entry_type) {
}

FuseEntry::~FuseEntry() {
}

FuseEntryType FuseEntry::get_entry_type() const {
    return this->entry_type;
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
