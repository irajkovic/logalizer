#pragma once

#include <chrono>
#include <string>

struct LogLine {

    //! Time when line was read.
    std::chrono::time_point<std::chrono::steady_clock> time;

    //! Actual contents.
    std::string text;

    //! Unique id of line, growing monotonically
    size_t id{0};

    //! Source of the line, or filter id if line matches filter
    uint8_t src{0};

    //! Object validity.
    //! Must be set to true fo the object to be considered
    //! to be properly constructed.
    bool valid{false};

    bool isValid() { return valid; }
};
