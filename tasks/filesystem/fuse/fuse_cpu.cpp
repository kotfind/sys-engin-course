#include "fuse_cpu.hpp"
#include "ctrl_file.hpp"
#include "fuse_dir.hpp"
#include "fuse_fs.hpp"
#include "unit.hpp"
#include "unit_file.hpp"

#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

static std::string_view get_ctrl_path() {
    return "ctrl";
}

static std::string get_unit_root_path(std::size_t unit_id) {
    return std::format("/unit{}", unit_id);
}

static std::string get_unit_data_path(std::size_t unit_id) {
    return std::format("/unit{}/lram", unit_id);
}

static std::string get_unit_program_path(std::size_t unit_id) {
    return std::format("/unit{}/pram", unit_id);
}

FuseCpu::FuseCpu(
    std::unique_ptr<FuseFs> fs, std::vector<std::unique_ptr<Unit>> units
)
    : fs(std::move(fs)), units(std::move(units)) {
}

static FuseDir* create_root_dir(std::size_t unit_count) {
    auto root_dir = std::make_unique<FuseDir>();

    for (std::size_t unit_id = 0; unit_id < unit_count; ++unit_id) {
        if (!root_dir->add_dir(
                get_unit_root_path(unit_id), std::make_unique<FuseDir>()
            )) {
            return nullptr;
        }

        if (!root_dir->add_file(
                get_unit_data_path(unit_id), std::make_unique<UnitFile>()
            )) {
            return nullptr;
        }

        if (!root_dir->add_file(
                get_unit_program_path(unit_id), std::make_unique<UnitFile>()
            )) {
            return nullptr;
        }
    }

    if (!root_dir->add_file(get_ctrl_path(), std::make_unique<CtrlFile>())) {
        return nullptr;
    }

    return root_dir.release();
}

static std::vector<std::unique_ptr<Unit>> create_units(std::size_t unit_count) {
    std::vector<std::unique_ptr<Unit>> units;
    for (std::size_t unit_id = 0; unit_id < unit_count; ++unit_id) {
        units.push_back(std::make_unique<Unit>(unit_id));
    }
    return units;
}

FuseCpu* FuseCpu::create(
    const std::filesystem::path& mount_path, std::size_t unit_count
) {
    auto units = create_units(unit_count);

    auto root = std::unique_ptr<FuseDir>(create_root_dir(unit_count));
    if (root == nullptr) {
        return nullptr;
    }

    auto fs =
        std::unique_ptr<FuseFs>(FuseFs::mount(mount_path, std::move(root)));
    if (fs == nullptr) {
        return nullptr;
    }

    return new FuseCpu(std::move(fs), std::move(units));
}
