#pragma once

#include <chrono>
#include <string>

struct LogLine {
    size_t row{0};
    size_t id{0};
    std::chrono::time_point<std::chrono::steady_clock> time;
    std::string text;
    bool valid{false};

    bool isValid() { return valid; }
};
