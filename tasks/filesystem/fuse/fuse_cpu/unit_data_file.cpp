#include "unit_data_file.hpp"
#include <format>

UnitDataFile::UnitDataFile(Cpu* cpu, std::size_t unit_id)
    : cpu(cpu), unit_id(unit_id) {
}

UnitDataFile::~UnitDataFile() {
}

std::string UnitDataFile::get_file_name_static(std::size_t unit_id) {
    return std::format("/unit{}/lram", unit_id);
}

void UnitDataFile::after_close(FuseFs* fs, std::string_view path) {
    auto data = this->move_write_data();
    if (data.empty()) {
        return;
    }

    this->cpu->set_data(this->unit_id, data);
    this->set_read_data(data, fs, path);
}

std::string UnitDataFile::get_file_name() const {
    return get_file_name_static(this->unit_id);
}
