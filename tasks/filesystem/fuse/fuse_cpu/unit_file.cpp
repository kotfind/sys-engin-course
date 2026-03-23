#include "unit_file.hpp"

#include "log.hpp"

UnitFile::~UnitFile() {
}

void UnitFile::after_close() {
    warn("closed unit file");
}
