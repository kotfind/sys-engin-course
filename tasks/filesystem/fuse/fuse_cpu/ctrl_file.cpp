#include "ctrl_file.hpp"

#include "cpu.hpp"
#include "log.hpp"

#include <string_view>

CtrlFile::CtrlFile(Cpu* cpu) : cpu(cpu) {
}

CtrlFile::~CtrlFile() {
}

void CtrlFile::after_close() {
    warn("TODO");
}

std::string_view CtrlFile::get_file_name() const {
    return "/ctrl";
}
