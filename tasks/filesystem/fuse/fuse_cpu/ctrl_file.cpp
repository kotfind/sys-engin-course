#include "ctrl_file.hpp"
#include "log.hpp"
#include <iterator>
#include <span>
#include <string_view>

CtrlFile::~CtrlFile() {
}

void CtrlFile::after_close() {
    warn("closed ctrl file");
}
