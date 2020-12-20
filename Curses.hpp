#pragma once

#include "Log.hpp"

class Curses {

    static const size_t _menuHeight = 3u;
    bool _active = false;
    std::vector<std::string> _tabs;

public:

    bool activate() {
        ::initscr();
        ::noecho();
        ::cbreak();
        ::keypad(stdscr, true);
        ::refresh();
        _active = has_colors();

        if (_active) {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_YELLOW, COLOR_BLACK);
            init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(5, COLOR_CYAN, COLOR_BLACK);
        }

        LOG("Curses: " << _active);

        return _active;
    }

    void addTab(std::string name, int index) {
        LOG(_tabs.size() << " " << index);
        if (_tabs.size() <= index) {
            _tabs.resize(index + 1u);
        }
        _tabs[index] = name;
    }

    void drawMenu() {
        size_t column = 2u;
        for (int i=1; i<_tabs.size(); i++) {
            attron(COLOR_PAIR(i));
            mvprintw(1, column, "[%d] %s", i, _tabs[i].c_str());
            column += _tabs[i].size() + 10u;
        }
    }
    
    void clear() {
        ::clear();
    }

    void refresh() {
        ::refresh();
    }

    void printLine(int row, std::string line, int index) {

        if (_active) {
            attron(COLOR_PAIR(index));
            mvprintw(row + _menuHeight, 0, "[%i] %s", index, line.c_str());
        }
    }

    ~Curses() {
        endwin();
    }
};

