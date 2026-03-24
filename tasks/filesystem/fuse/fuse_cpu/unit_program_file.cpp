#include "unit_program_file.hpp"

#include <format>
#include <string_view>

UnitProgramFile::UnitProgramFile(Cpu* cpu, std::size_t unit_id)
    : cpu(cpu), unit_id(unit_id) {
}

UnitProgramFile::~UnitProgramFile() {
}

void UnitProgramFile::after_close() {
    auto data = this->move_write_data();
    if (data.empty()) {
        return;
    }

    auto code = std::string_view((const char*)data.data(), data.size());
    this->cpu->set_program_code(this->unit_id, code);
    this->set_read_data(data);
}

std::string UnitProgramFile::get_file_name() const {
    return std::format("/unit{}/pram", this->unit_id);
}
