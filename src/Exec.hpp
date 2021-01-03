#pragma once

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "Log.hpp"

std::string exec(const std::string& cmd, const std::string& line) {

    std::string fullCmd = cmd + " '" + line + "'"; 
    std::array<char, 128> buffer;
    std::string result;

    LOG("Executing: " << fullCmd);

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCmd.c_str(), "r"), pclose);

    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    return result;
}
