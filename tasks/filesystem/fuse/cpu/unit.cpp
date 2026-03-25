#include "unit.hpp"

#include "log.hpp"
#include "program.hpp"

#include <functional>
#include <future>
#include <iterator>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

Unit::Unit(std::size_t unit_id)
    : id(unit_id), program(Program::dummy()), is_running(false) {
    info("Created unit {}", unit_id);
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

bool Unit::set_program_code(std::string_view source_code) {
    auto lock = this->wait_until_finished();

    auto* new_program = Program::compile(source_code);
    if (new_program != nullptr) {
        this->program.reset(new_program);
        info("Updated program for unit {}", this->id);
        return true;
    } else {
        this->program.reset(Program::dummy());
        error("Failed to update program for unit {}", this->id);
        return false;
    }
}

void Unit::set_data(std::span<const std::byte> data) {
    auto lock = this->wait_until_finished();

    info("Loaded data for unit {}", this->id);

    this->data.assign(std::begin(data), std::end(data));
}

std::vector<std::byte> Unit::copy_data() const {
    auto lock = this->wait_until_finished();

    return this->data;
}

std::vector<std::byte> Unit::move_data() {
    auto lock = this->wait_until_finished();

    std::vector<std::byte> ans;
    this->data.swap(ans);

    return ans;
}

int Unit::get_status_code() const {
    auto lock = this->wait_until_finished();

    return this->status_code;
}

std::future<Unit*> Unit::run(std::function<void(Unit*)> on_start) {
    auto lock = this->wait_until_finished();

    info("Running program on unit {}", this->id);

    this->is_running = true;

    std::thread([this, on_start]() { on_start(this); }).detach();

    return std::async([this] {
        auto status = this->program->run(this->data);

        std::lock_guard lock(this->mutex);

        this->status_code = status;

        this->is_running = false;
        this->is_running_cv.notify_all();

        if (this->status_code == 0) {
            success("Program finished with status=0 on unit {}", this->id);
        } else {
            error(
                "Program finished with non-zero status={} on unit {}",
                this->status_code,
                this->id
            );
        }

        return this;
    });
}
