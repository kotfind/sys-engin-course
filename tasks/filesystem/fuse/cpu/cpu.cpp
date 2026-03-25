#include "cpu.hpp"
#include "unit.hpp"

#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>

Cpu::Cpu(std::size_t unit_count) {
    for (std::size_t unit_id = 0; unit_id < unit_count; ++unit_id) {
        this->units.push_back(std::make_unique<Unit>(unit_id));
    }
}

std::size_t Cpu::get_unit_count() const {
    return this->units.size();
}

void Cpu::set_program_code(std::size_t unit_id, std::string_view source_code) {
    std::lock_guard lock(this->mutex);

    this->units.at(unit_id)->set_program_code(source_code);
}

void Cpu::set_data(std::size_t unit_id, std::span<const std::byte> data) {
    std::lock_guard lock(this->mutex);

    this->units.at(unit_id)->set_data(data);
}

std::vector<std::byte> Cpu::move_data(std::size_t unit_id) {
    std::lock_guard lock(this->mutex);

    return this->units.at(unit_id)->move_data();
}

int Cpu::get_status_code(std::size_t unit_id) {
    std::lock_guard lock(this->mutex);

    return this->units.at(unit_id)->get_status_code();
}

std::future<std::size_t> Cpu::run(
    std::size_t unit_id, std::function<void(std::size_t)> on_start
) {
    std::lock_guard lock(this->mutex);

    auto* unit = this->units.at(unit_id).get();

    auto on_start_wrapped = [on_start](Unit* unit) {
        on_start(unit->get_id());
    };

    return std::async([unit, on_start_wrapped]() {
        return unit->run(on_start_wrapped).get()->get_id();
    });
}

bool Cpu::get_is_running(std::size_t unit_id) const {
    std::lock_guard lock(this->mutex);

    return this->units.at(unit_id)->get_is_running();
}
