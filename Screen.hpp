#pragma once

#include <functional>
#include <vector>
#include <regex>

#include "Log.hpp"
#include "LogLine.hpp"

struct Tab {
    std::string name;
    bool enabled;
};

struct Filter {
    int id;
    std::string name;
    std::regex regex;
};

class Screen {

public:

    void prepareLines() {
        _nextLine = _row;    
    }

    LogLine* nextLine() {

        // Skip lines from disabled tabs
        while ((_nextLine < _lines.size())
                && !_tabs[_lines[_nextLine].id].enabled) {
            _nextLine++;
        }

        if (_nextLine < _lines.size()) {
            return &_lines[_nextLine++];
        }

        return nullptr;
    }

    void toggleTab(int id) {
        _tabs[id].enabled = !_tabs[id].enabled;
    }

    Tab* getTab(int id) {
        if (id < _tabs.size()) {
            return &_tabs[id];
        }
        return nullptr;
    }

    size_t getTabCnt() {
        return _tabs.size();
    }

    void scrollUp() {
        LOG("row " << _row);
        std::lock_guard<std::mutex> g(_mtx);
        if (_row > 0) {
            --_row;
        }
    }

    void scrollDown() {
        std::lock_guard<std::mutex> g(_mtx);
        LOG("row " << _row << ", size=" << _lines.size());
        if (_row < _lines.size() - 1) {
            ++_row;
        }
    }

    void registerOnNewDataAvailableListener(std::function<void()> listener) {
        _onNewDataAvailable = listener;
    }

    void addFilter(const std::string& name, const std::string& regex) {
        _filters.emplace_back(Filter{_id++, name, std::regex{regex}});
        _tabs.emplace_back(Tab{name, true});
    }

    std::function<void(std::string)> getAppender(std::string name) {

        auto id = _id++;

        LOG("Appender " << name << " (" << id << ")");
        _tabs.emplace_back(Tab{name, true});

        return [this, id] (std::string line) {
            addLine(line, id);
        };
    }

    void addLine(const std::string& text, int id) {
        {
            LogLine line{std::chrono::steady_clock::now(), text, id};

            // Matching filter takes the ownership of the line.
            for (const auto& filter : _filters) {
                if (std::regex_match(text, filter.regex)) {
                    LOG("Line matches regex: " << text);
                    line.id = filter.id;
                    break;
                }
            }

            {
                std::lock_guard<std::mutex> g(_mtx);
                _lines.emplace_back(line);
            }
            
            if (_onNewDataAvailable) {
                _onNewDataAvailable();
            }
        }
    }


private:
    std::mutex  _mtx;
    int         _id = 0;
    size_t      _row = 0;
    size_t      _nextLine = 0;
    std::vector<LogLine> _lines;
    std::vector<Tab> _tabs;
    std::vector<Filter> _filters;
    std::function<void()> _onNewDataAvailable;
};

