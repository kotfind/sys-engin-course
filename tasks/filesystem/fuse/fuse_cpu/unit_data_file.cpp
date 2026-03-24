#include "unit_data_file.hpp"
#include <format>

UnitDataFile::UnitDataFile(Cpu* cpu, std::size_t unit_id)
    : cpu(cpu), unit_id(unit_id) {
}

UnitDataFile::~UnitDataFile() {
}

void UnitDataFile::after_close() {
    auto data = this->move_write_data();
    if (data.empty()) {
        return;
    }

    this->cpu->set_data(this->unit_id, data);
    this->set_read_data(data);
}

std::string UnitDataFile::get_file_name() const {
    return std::format("/unit{}/lram", this->unit_id);
}
