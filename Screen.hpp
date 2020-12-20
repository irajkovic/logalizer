#pragma once

#include "Log.hpp"
#include "LogLine.hpp"

class Screen {

    static const size_t _height = 10;
public:

    Screen(Curses& curses) : _curses(curses) {}

    void redraw() {

        _curses.clear();
        _curses.drawMenu();

        std::lock_guard<std::mutex> g(_mtx);

        for (int i=0; i<_height; i++) {
            int lineInd = _row + i;
            if (lineInd > _lines.size()) {
                break;
            }
            auto line = _lines[lineInd];
            _curses.printLine(i, line.text, line.sourceId);
        }

        _curses.refresh();
    }

    void toggle(int index) {

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

    std::function<void(std::string)> getAppender(std::string name, int index) {

        LOG("Appender " << name << " (" << index << ")");
        _curses.addTab(name, index);

        return [this, index] (std::string line) {
            addLine(line, index);
        };
    }

    void addLine(const std::string& text, int index) {
        {
            std::lock_guard<std::mutex> g(_mtx);
            _lines.emplace_back(LogLine{std::chrono::steady_clock::now(), text, index});
            LOG("Added line, size=" << _lines.size());
        }
    }


private:
    Curses& _curses;
    std::mutex  _mtx;
    size_t      _row = 0;
    std::vector<LogLine> _lines;
};

