#include "../common/format.h"

#include <iomanip>
#include <sstream>

std::string Format::date(const std::chrono::time_point<std::chrono::system_clock>& date)
{
    const std::time_t now = std::chrono::system_clock::to_time_t(date);
    const tm* time_info = localtime(&now);

    std::string s(26, ' ');
    std::strftime(s.data(), s.size(), "%Y-%m-%d %H:%M", time_info);
    const auto pos = s.find_last_not_of(' ');
    s.erase(pos, s.size() - pos);

    return s;
}

std::string Format::timeDuration(const size_t millisecond_duration)
{
    const size_t hours = millisecond_duration / 3600000;
    const size_t minutes = (millisecond_duration % 3600000) / 60000;
    const size_t seconds = (millisecond_duration % 60000) / 1000;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << seconds;

    return ss.str();
}

std::string Format::progress(double progress)
{
    std::string d_str = std::to_string(progress);
    const size_t dot_pos = d_str.find('.');
    std::string left, right;
    if (dot_pos != std::string::npos)
    {
        left = d_str.substr(0, dot_pos);
        right = d_str.substr(dot_pos + 1);
    }
    else
    {
        left = d_str;
    }
    const size_t right_len = (4 - left.length());
    const std::string r_n = right.length() >= right_len ? right.substr(0, right_len) : right + std::string(
                                    static_cast<int>(right_len - right.length()), ' ');

    return left + "." + r_n + "%";
}

std::string Format::largeNumber(size_t n)
{
    std::string int_string = std::to_string(n);
    size_t pos = int_string.length() - 3;
    while (pos > 0 && pos < int_string.length())
    {
        int_string.insert(pos, " ");
        pos -= 3;
    }
    return int_string;
}
