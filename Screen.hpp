#pragma once

#include <functional>
#include <vector>
#include <regex>

#include "Log.hpp"
#include "LogLine.hpp"

struct Tab {
    std::string name;
    bool enabled;
    size_t rowsCnt{0};
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

        // TODO :: Use locking algorithm
        std::lock_guard<std::mutex> g1(_tabMtx);
        std::lock_guard<std::mutex> g2(_mtx);

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
        std::lock_guard<std::mutex> g(_tabMtx);
        _tabs[id].enabled = !_tabs[id].enabled;
    }

    Tab* getTab(int id) {
        std::lock_guard<std::mutex> g(_tabMtx);
        if (id < _tabs.size()) {
            return &_tabs[id];
        }
        return nullptr;
    }

    size_t getTabCnt() {
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

    void registerOnNewDataAvailableListener(std::function<void()> listener) {
        _onNewDataAvailable = listener;
    }

    void addFilter(const std::string& name, const std::string& regex) {
        LOG("Filter " << name << " (" << _id << ")");
        std::lock_guard<std::mutex> g(_tabMtx);
        _filters.emplace_back(Filter{_id++, name, std::regex{regex}});
        _tabs.emplace_back(Tab{name, true, 0});
    }

    std::function<void(std::string)> getAppender(std::string name) {

        auto id = _id++;

        LOG("Appender " << name << " (" << id << ")");
        std::lock_guard<std::mutex> g(_tabMtx);
        _tabs.emplace_back(Tab{name, true});

        return [this, id] (std::string line) {
            addLine(line, id);
        };
    }

    void addLine(const std::string& text, int id) {

        LogLine line{std::chrono::steady_clock::now(), text, id};

        // Matching filter takes the ownership of the line.
         
        {
            std::lock_guard<std::mutex> g(_tabMtx);
            for (const auto& filter : _filters) {
                if (std::regex_match(text, filter.regex)) {
                    line.id = filter.id;
                    break;
                }
            }
        }

        {
            std::lock_guard<std::mutex> g(_mtx);
            _lines.emplace_back(line);
            _tabs[line.id].rowsCnt++;
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
    int         _id = 0;
    size_t      _row = 0;
    size_t      _nextLine = 0;
    std::vector<LogLine> _lines;
    std::vector<Tab> _tabs;
    std::vector<Filter> _filters;
    std::function<void()> _onNewDataAvailable;
};

