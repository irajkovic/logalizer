#pragma once

#include <chrono>
#include <string>

struct LogLine {
    std::chrono::time_point<std::chrono::steady_clock> time;
    std::string text;
    int id;
};

