#pragma once

#include <functional>

#include "IExec.hpp"

struct ExecMock : public IExec {

    using ExecHandler = std::function<std::string(const std::string&, const std::string&)>;

    ExecHandler execHandler;

    std::string exec(const std::string& command, const std::string& line) override {
        return execHandler ? execHandler(command, line) : "";
    }
};
