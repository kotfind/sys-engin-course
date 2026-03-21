#pragma once

#include <cstdint>
#include <functional>

std::size_t get_page_size();

double with_nanos_duration(const std::function<void()>& func);

void write_point(double x, double y);
