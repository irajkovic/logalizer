#pragma once

#include <cassert>
#include <chrono>
#include <functional>
#include <vector>
#include <regex>
#include <string>
#include <limits>
#include <map>

#include "Log.hpp"
#include "LogLine.hpp"
#include "Exec.hpp"

struct Tab {
    std::string name;
    bool enabled;
    size_t rowsCnt{0};
    bool valid{false};

    operator bool() {
        return valid;
    }
};

struct Filter {
    int src;
    std::string name;
    std::regex regex;
    std::string command;
};

class IDataModel {

public:

    virtual bool scrollUp() = 0;

    virtual bool scrollDown() = 0;

    virtual void prepareLines() = 0;

    virtual LogLine nextLine() = 0;

    virtual void toggleTab(uint8_t src) = 0;

    virtual Tab getTab(uint8_t src) = 0;

    virtual uint8_t getTabCnt() = 0;

    virtual void registerOnNewDataAvailableListener(
        std::function<void()> listener) = 0;

    virtual void addFilter(
        const std::string& name, const std::string& regex) = 0;

    virtual bool addExternal(
        const std::string& name, const std::string& command) = 0;

    virtual std::function<void(std::string)> getAppender(
        std::string name) = 0;

    virtual void addLine(const std::string& text, uint8_t src) = 0;
};

