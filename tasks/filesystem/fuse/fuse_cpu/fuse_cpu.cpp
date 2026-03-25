#include "fuse_cpu.hpp"
#include "cpu.hpp"
#include "ctrl_file.hpp"
#include "fuse_dir.hpp"
#include "fuse_fs.hpp"
#include "unit_data_file.hpp"
#include "unit_program_file.hpp"

#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <utility>

static std::string get_unit_root_path(std::size_t unit_id) {
    return std::format("/unit{}", unit_id);
}

FuseCpu::FuseCpu(std::unique_ptr<FuseFs> fs, std::unique_ptr<Cpu> cpu)
    : fs(std::move(fs)), cpu(std::move(cpu)) {
}

static FuseDir* create_root_dir(Cpu* cpu) {
    auto root = std::make_unique<FuseDir>();

    for (std::size_t unit_id = 0; unit_id < cpu->get_unit_count(); ++unit_id) {
        if (!root->add_dir(
                get_unit_root_path(unit_id), std::make_unique<FuseDir>()
            )) {
            return nullptr;
        }

        auto prog_file = std::make_unique<UnitProgramFile>(cpu, unit_id);
        if (!root->add_file(prog_file->get_file_name(), std::move(prog_file))) {
            return nullptr;
        }

        auto data_file = std::make_unique<UnitDataFile>(cpu, unit_id);
        if (!root->add_file(data_file->get_file_name(), std::move(data_file))) {
            return nullptr;
        }
    }

    auto ctrl_file = std::make_unique<CtrlFile>(cpu);
    if (!root->add_file(ctrl_file->get_file_name(), std::move(ctrl_file))) {
        return nullptr;
    }

    return root.release();
}

FuseCpu* FuseCpu::create(
    const std::filesystem::path& mount_path, std::size_t unit_count
) {
    auto cpu = std::make_unique<Cpu>(unit_count);

    auto root = std::unique_ptr<FuseDir>(create_root_dir(cpu.get()));
    if (root == nullptr) {
        return nullptr;
    }

    auto fs =
        std::unique_ptr<FuseFs>(FuseFs::mount(mount_path, std::move(root)));
    if (fs == nullptr) {
        return nullptr;
    }

    return new FuseCpu(std::move(fs), std::move(cpu));
}
