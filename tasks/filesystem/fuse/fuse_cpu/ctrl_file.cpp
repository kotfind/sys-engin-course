#include "ctrl_file.hpp"

#include "cpu.hpp"
#include "fuse_fs.hpp"
#include "log.hpp"
#include "unit_data_file.hpp"

#include <charconv>
#include <cstddef>
#include <span>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

CtrlFile::CtrlFile(Cpu* cpu) : cpu(cpu) {
}

CtrlFile::~CtrlFile() {
}

std::vector<std::size_t> parse_unit_ids(
    const std::string& str_data, std::size_t unit_count
) {
    std::stringstream ss(str_data);

    std::vector<std::size_t> units_to_run;
    std::string unit_id_str;
    while (ss >> unit_id_str) {
        if (unit_id_str.empty()) {
            continue;
        }

        std::size_t unit_id;
        auto res = std::from_chars(
            unit_id_str.data(), unit_id_str.data() + unit_id_str.size(), unit_id
        );
        if (res.ec != std::errc()) {
            error(
                "Failed to parse `{}` as an unit_id in /ctrl input: "
                "will ignore the whole request ",
                unit_id_str
            );
            return {};
        }

        if (unit_id >= unit_count) {
            error(
                "Unit {} doesn't exist: will ignore the whole request", unit_id
            );
            return {};
        }

        units_to_run.push_back(unit_id);
    }

    return units_to_run;
}

void print_units_to_run(std::span<std::size_t> unit_ids) {
    std::stringstream ss;
    bool is_first = true;
    for (auto unit_id : unit_ids) {
        if (is_first) {
            is_first = false;
        } else {
            ss << ", ";
        }
        ss << unit_id;
    }
    info("Going to run the following units: {}", ss.str());
}

void CtrlFile::after_close(FuseFs* fs, std::string_view path) {
    (void)path;

    auto data = this->move_write_data();
    if (data.empty()) {
        return;
    }

    auto str_data = std::string((char*)data.data(), data.size());
    auto units_to_run = parse_unit_ids(str_data, this->cpu->get_unit_count());
    if (units_to_run.empty()) {
        return;
    }

    print_units_to_run(units_to_run);

    for (auto unit_id : units_to_run) {
        if (this->cpu->get_is_running(unit_id)) {
            warn("Unit {} is already running", unit_id);
            continue;
        }

        auto* cpu = this->cpu;
        std::thread([cpu, unit_id, fs]() {
            cpu->run(unit_id).wait();
            auto file_name = UnitDataFile::get_file_name_static(unit_id);
            auto status =
                fs->set_file_read_data(file_name, cpu->move_data(unit_id));
            if (!status) {
                error("failed to set data for {}", file_name);
            }
        }).detach();
    }
}

std::string_view CtrlFile::get_file_name() const {
    return "/ctrl";
}
