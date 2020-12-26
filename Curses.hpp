#pragma once

#include <atomic>
#include <curses.h>
#include <mutex>

#include "Screen.hpp"
#include "Log.hpp"

class Curses {

    int _menuHeight = 0;
    std::atomic<bool> _active{false};
    std::vector<std::string> _tabs;
    Screen& _screen;
    std::atomic<bool> _drawing{false};

public:

    Curses(Screen& screen) : _screen(screen) {
        screen.registerOnNewDataAvailableListener([this](){
            redraw();
        });
    }

    bool activate() {
        ::initscr();
        ::noecho();
        ::cbreak();
        ::keypad(stdscr, true);
        _active = has_colors();

        if (_active) {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);
            init_pair(2, COLOR_YELLOW, COLOR_BLACK);
            init_pair(3, COLOR_GREEN, COLOR_BLACK);
            init_pair(4, COLOR_CYAN, COLOR_BLACK);
            init_pair(5, COLOR_BLUE, COLOR_BLACK);
            init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(7, COLOR_RED, COLOR_BLACK);

            init_pair(8,  COLOR_BLACK, COLOR_WHITE);
            init_pair(9, COLOR_BLACK, COLOR_YELLOW);
            init_pair(10, COLOR_BLACK, COLOR_GREEN);
            init_pair(11, COLOR_BLACK, COLOR_CYAN);
            init_pair(12, COLOR_BLACK, COLOR_BLUE);
            init_pair(13, COLOR_BLACK, COLOR_MAGENTA);
            init_pair(14, COLOR_BLACK, COLOR_RED);
        }

        LOG("Curses: " << _active);

        return _active;
    }

    bool run() {

        if (!activate()) {
            return false;
        }

        while (true) {
            redraw();
            int ch = getch();
            switch (ch) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    _screen.toggleTab(ch - '0');
                    break;
                case KEY_UP:
                    _screen.scrollUp();
                    break;
                case KEY_DOWN:
                    _screen.scrollDown();
                    break;
                case 'q':
                case 'Q':
                    _active = false;
                    return true;
                    break;
            }
        }

        return true;
    }

    void addTab(std::string name, int index) {
        LOG(_tabs.size() << " " << index);
        if (_tabs.size() <= index) {
            _tabs.resize(index + 1u);
        }
        _tabs[index] = name;
    }

    std::string getTabTitle(const Tab& tab, const uint8_t src) {
        const int kMaxNameLength = 16;
        std::stringstream stream;

        std::string tabName = tab.name;;
        if (tabName.length() > kMaxNameLength) {
            tabName = tabName.substr(tabName.length() - kMaxNameLength, std::string::npos);
        }

        stream << "[" << static_cast<int>(src) << "] "  << tabName << " (" << tab.rowsCnt << ")";
        return stream.str();
    }

    int drawMenu(int maxcol) {

        static const int kHorMargin = 2;
        static const int kVerMargin = 0;
        static const int kVerPadding = 2;

        int row = kVerMargin;
        int column = kHorMargin;

        for (int i = 0; i < _screen.getTabCnt(); i++) {

            auto tab = _screen.getTab(i);

            if (tab == nullptr) {
                return row + kVerPadding;
            }

            tab->enabled ? attron(COLOR_PAIR(i+8)) : attron(COLOR_PAIR(i+1));

            auto tabTitle = getTabTitle(*tab, i);

            if (column + tabTitle.length() > maxcol) {
                column = kHorMargin;
                row += kVerPadding;
            }

            mvprintw(row, column, "%s", tabTitle.c_str());
            column += tabTitle.length() + kHorMargin;
        }

        return row + kVerPadding;
    }
    
    void redraw() {

        if (!_active) {
            return;
        }

        bool expected = false;
        if (!_drawing.compare_exchange_weak(expected, true)) {
            return;
        }

        int screenWidth, screenHeight;
        getmaxyx(stdscr, screenWidth, screenHeight);
       
        ::clear();
        _menuHeight = drawMenu(screenHeight);
        _screen.prepareLines(screenHeight);

        for (size_t i = 0; i < screenHeight; i++) {
            auto line = _screen.nextLine();

            if (!line.isValid()) {
                LOG("Line not valid: " << line.id);
                break;
            }

            printLine(i, line);
        }

        ::refresh();
        _drawing = false;
    }

    void printLine(int row, const LogLine& line) {
        if (_active) {
            attron(COLOR_PAIR(line.src + 1));
            mvprintw(row + _menuHeight, 0, "%6d [%d] %s", line.id, line.src, line.text.c_str());
        }
    }

    ~Curses() {
        endwin();
    }
};

