#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <regex>
#include <string>

#include "Log.hpp"
#include "LogLine.hpp"

struct Tab {
    std::string name;
    bool enabled;
    size_t rowsCnt{0};
};

struct Filter {
    int src;
    std::string name;
    std::regex regex;
};

class Screen {

    struct LogLineInternal {
        std::chrono::time_point<std::chrono::steady_clock> time;
        std::string text;
        uint8_t src;
    };

public:

    void prepareLines(int needed) {
    #if 0
        auto maxLines = getLinesCntWithFilters();
        if (_row >= maxLines - needed) {
            row = maxLinues - needed;
        }
    #endif
        _nextLine = _row;    
    }

    LogLine nextLine() {

        // TODO :: Use locking algorithm
        std::lock_guard<std::mutex> g1(_tabMtx);
        std::lock_guard<std::mutex> g2(_mtx);

        // Skip lines from disabled tabs
        while ((_nextLine < _lines.size())
                && !_tabs[_lines[_nextLine].src].enabled) {
            _nextLine++;
        }

        if (_nextLine < _lines.size()) {
            const auto& line = _lines[_nextLine];
            return LogLine{line.time, line.text, _nextLine++, line.src, true};
        }

        return {};
    }

    void toggleTab(uint8_t src) {
        std::lock_guard<std::mutex> g(_tabMtx);
        _tabs[src].enabled = !_tabs[src].enabled;
    }

    Tab* getTab(uint8_t src) {
        std::lock_guard<std::mutex> g(_tabMtx);
        if (src < _tabs.size()) {
            return &_tabs[src];
        }
        return nullptr;
    }

    uint8_t getTabCnt() {
        std::lock_guard<std::mutex> g(_tabMtx);
        return _tabs.size();
    }

    void scrollUp() {
        std::lock_guard<std::mutex> g(_mtx);
        if (_row > 0) {
            --_row;
        }
    }

    void scrollDown() {
        std::lock_guard<std::mutex> g(_mtx);
        auto maxLines = getLinesCntWithFilters();
        _row = (_row < maxLines) ? (_row + 1) : (0);
    }

    size_t getRow() {
        return _row;
    }

    void registerOnNewDataAvailableListener(std::function<void()> listener) {
        _onNewDataAvailable = listener;
    }

    void addFilter(const std::string& name, const std::string& regex) {
        LOG("Filter " << name << " (" << _src << ")");
        std::lock_guard<std::mutex> g(_tabMtx);
        _filters.emplace_back(Filter{_src++, name, std::regex{regex}});
        _tabs.emplace_back(Tab{name, true, 0});
    }

    std::function<void(std::string)> getAppender(std::string name) {

        auto src = _src++;

        LOG("Appender " << name << " (" << src << ")");
        std::lock_guard<std::mutex> g(_tabMtx);
        _tabs.emplace_back(Tab{name, true});

        return [this, src] (std::string line) {
            addLine(line, src);
        };
    }

    void addLine(const std::string& text, uint8_t src) {

        LogLineInternal line{std::chrono::steady_clock::now(), text, src};

        // Matching filter takes the ownership of the line.
         
        {
            std::lock_guard<std::mutex> g(_tabMtx);
            for (const auto& filter : _filters) {
                if (std::regex_match(text, filter.regex)) {
                    line.src = filter.src;
                    break;
                }
            }
        }

        {
            std::lock_guard<std::mutex> g(_mtx);
            _lines.emplace_back(line);
            _tabs[line.src].rowsCnt++;
        }
        
        if (_onNewDataAvailable) {
            _onNewDataAvailable();
        }
    }

    size_t getLinesCntWithFilters() {
        std::lock_guard<std::mutex> g(_tabMtx);
        size_t cnt = 0;
        for (const auto& tab : _tabs) {
            if (tab.enabled) {
                cnt += tab.rowsCnt;
            }
        }
        return cnt;
    }

private:
    std::mutex  _mtx;
    std::mutex  _tabMtx;
    int         _src = 0;
    size_t      _row = 0;
    size_t      _nextLine = 0;
    std::vector<LogLineInternal> _lines;
    std::vector<Tab> _tabs;
    std::vector<Filter> _filters;
    std::function<void()> _onNewDataAvailable;
};

