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

#include "IDataModel.hpp"

namespace {
    const size_t kUndefined = std::numeric_limits<size_t>::max();
};

class DataModel : public IDataModel {

    struct LogLineInternal {
        std::chrono::time_point<std::chrono::steady_clock> time;
        std::string text;
        uint8_t src;
    };

    struct TabInternal {
        std::string name;
        bool enabled;
        size_t rowsCnt{0};
        size_t minLineId{kUndefined};
        size_t maxLineId{kUndefined};
    };

public:

    bool scrollUp() {

        std::lock_guard<std::mutex> g(_mtx);
        return reverseFiltered(&_row);
    }

    bool scrollDown() {

        std::lock_guard<std::mutex> g(_mtx);
        return fastForwardFiltered(&_row);
    }

    void prepareLines() {

        std::lock_guard<std::mutex> g(_mtx);
        _nextLine = clampRow(_row);
        _hasNextLine = _nextLine != kUndefined;
    }

    LogLine nextLine() {

        std::lock_guard<std::mutex> g(_mtx);

        if (!_hasNextLine) {
            return {};
        }

        assert(_nextLine < _lines.size());

        const auto& internal = _lines[_nextLine];
        LogLine line{internal.time, internal.text, "", _nextLine, internal.src, true};

        auto comment = _comments.find(_nextLine);
        if (comment != std::end(_comments)) {
            line.comment = comment->second;
        }
    
        _hasNextLine = fastForwardFiltered(&_nextLine);
        
        return line;
    }

    void toggleTab(uint8_t src) {
        std::lock_guard<std::mutex> g(_mtx);
        _tabs[src].enabled = !_tabs[src].enabled;
    }

    Tab getTab(uint8_t src) {
        std::lock_guard<std::mutex> g(_mtx);
        if (src < _tabs.size()) {
            TabInternal& tab = _tabs[src];
            return Tab{tab.name, tab.enabled, tab.rowsCnt, true};
        }
        return {};
    }

    uint8_t getTabCnt() {
        std::lock_guard<std::mutex> g(_mtx);
        return _tabs.size();
    }

    void registerOnNewDataAvailableListener(std::function<void()> listener) {
        _onNewDataAvailable = listener;
    }

    void addFilter(const std::string& name, const std::string& regex) {

        LOG("Filter " << name << " (" << _src << ")");
        std::lock_guard<std::mutex> g(_mtx);
        _filters.emplace_back(Filter{_src++, name, std::regex{regex}});
        _tabs.emplace_back(TabInternal{name, true, 0, kUndefined});
    }

    bool addExternal(const std::string& name, const std::string& command) {

        std::lock_guard<std::mutex> g(_mtx);
        for (auto& filter : _filters) {
            if (filter.name == name) {
                filter.command = command;
                return true;
            }
        }
        return false;
    }

    std::function<void(std::string)> getAppender(std::string name) {

        std::lock_guard<std::mutex> g(_mtx);
        auto src = _src++;
        LOG("Appender " << name << " (" << src << ")");
        _tabs.emplace_back(TabInternal{name, true});

        return [this, src] (std::string line) {
            addLine(line, src);
        };
    }

    void addLine(const std::string& text, uint8_t src) {

        LogLineInternal line{std::chrono::steady_clock::now(), text, src};
        const std::string *command = nullptr;

        // Matching filter takes the ownership of the line.
        {
            std::lock_guard<std::mutex> g(_mtx);

            for (const auto& filter : _filters) {
                if (std::regex_match(text, filter.regex)) {
                    line.src = filter.src;

                    if (!filter.command.empty()) {
                        command = &filter.command;
                    }

                    break;
                }
            }

            auto lineId = _lines.size();
            _lines.emplace_back(line);

            // Update tabs
            _tabs[line.src].rowsCnt++;
            _tabs[line.src].maxLineId = lineId;
            if (_tabs[line.src].minLineId == kUndefined) {
                _tabs[line.src].minLineId = lineId;
            }

            // Run command
            if (command) {
                _comments[lineId] = exec(*command, text);
                LOG("Added comment (" << lineId << ") "  << _comments[lineId]);
            }
        }
        
        if (_onNewDataAvailable) {
            _onNewDataAvailable();
        }
    }

private:

    size_t clamp(size_t wanted, size_t min, size_t max) {
        if (wanted < min) {
            return min;
        }
        else if (wanted > max) {
            return max;
        }
        else {
            return wanted;
        }
    }

    size_t clampRow(size_t wanted) {
        return clamp(wanted, getMinLineWithFilters(), getMaxLineWithFilters());
    }

    bool reverseFiltered(size_t *from) {
        size_t i = *from - 1u;
        while ((i < _lines.size()) && !_tabs[_lines[i].src].enabled) {
            i--;
        }

        if (i < _lines.size()) {
            *from = i;
            return true;
        }

        return false;
    }

    size_t fastForwardFiltered(size_t *from) {
        size_t i = *from + 1u;
        while ((i < _lines.size()) && !_tabs[_lines[i].src].enabled) {
            i++;
        }

        if (i < _lines.size()) {
            *from = i;
            return true;
        }

        return false;
    }

    size_t getMinLineWithFilters() {
        size_t minLineId = kUndefined;
        bool found = false;
        for (const auto& tab : _tabs) {
            if (tab.enabled && (tab.minLineId < minLineId)) {
                minLineId = tab.minLineId;
                found = true;
            }
        }
        return found ? minLineId : kUndefined;    
    }

    size_t getMaxLineWithFilters() {
        size_t maxLineId = 0u;
        bool found = false;
        for (const auto& tab : _tabs) {
            if (tab.enabled && (tab.maxLineId > maxLineId)) {
                maxLineId = tab.maxLineId;
                found = true;
            }
        }
        return found ? maxLineId : kUndefined;    
    }

    size_t getLinesCntWithFilters() {
        size_t cnt = 0;
        for (const auto& tab : _tabs) {
            if (tab.enabled) {
                cnt += tab.rowsCnt;
            }
        }
        return cnt;
    }

    std::mutex  _mtx;
    int         _src = 0;
    size_t      _row = 0;
    size_t      _nextLine = 0;
    bool        _hasNextLine = false;
    std::vector<LogLineInternal> _lines;
    std::vector<TabInternal> _tabs;
    std::vector<Filter> _filters;
    std::function<void()> _onNewDataAvailable;
    std::map<size_t, std::string> _comments;
};

