#pragma once

#include <string>

struct IExec {
    virtual std::string exec(const std::string& cmd, const std::string& line) = 0;
};

