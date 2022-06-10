#pragma once

#include <chrono>
#include <string>

namespace Format
{
    std::string date(const std::chrono::time_point<std::chrono::system_clock>& date);
    std::string time_duration(size_t millisecond_duration);
    std::string large_number(size_t n);
    std::string progress(double progress);
}
