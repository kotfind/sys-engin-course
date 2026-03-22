#include "unit.hpp"

#include "dynamic_program.hpp"

#include <memory>
#include <mutex>

Unit::Unit(std::size_t unit_id)
    : id(unit_id), program(DynamicProgram::dummy()) {
}

std::unique_lock<std::mutex> Unit::wait_until_finished() const {
    std::unique_lock lock(this->mutex);
    this->is_running_cv.wait(lock, [this]() { return !this->is_running; });
    return lock;
}

std::size_t Unit::get_id() const {
    return this->id;
}

bool Unit::get_is_running() const {
    std::lock_guard lock(this->mutex);

    return this->is_running;
}

bool Unit::set_program_code(const std::string& source_code) {
    auto lock = this->wait_until_finished();

    auto* new_program = DynamicProgram::compile(source_code);
    if (new_program != nullptr) {
        this->program.reset(new_program);
        return true;
    } else {
        this->program.reset(DynamicProgram::dummy());
        return false;
    }
}

void Unit::set_data(const std::vector<std::uint8_t>& data) {
    auto lock = this->wait_until_finished();

    this->data = data;
}

std::vector<std::uint8_t> Unit::get_data() const {
    auto lock = this->wait_until_finished();

    return this->data;
}

int Unit::get_status_code() const {
    auto lock = this->wait_until_finished();

    return this->status_code;
}

std::future<Unit*> Unit::run() {
    auto lock = this->wait_until_finished();

    this->is_running = true;

    return std::async([this] {
        this->program->run(this->data.size(), this->data.data());

        std::lock_guard lock(this->mutex);

        this->is_running = false;
        this->is_running_cv.notify_all();

        return this;
    });
}
