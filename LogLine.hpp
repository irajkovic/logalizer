#pragma once

struct LogLine {
    std::chrono::time_point<std::chrono::steady_clock> time;
    std::string text;
    size_t sourceId;
};

