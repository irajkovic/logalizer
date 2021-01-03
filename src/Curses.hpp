#pragma once

#include <atomic>
#include <curses.h>
#include <mutex>

#include "Screen.hpp"
#include "Log.hpp"

class Curses {

    std::atomic<bool> _active{false};
    std::vector<std::string> _tabs;
    Screen& _screen;
    std::atomic<bool> _drawing{false};
    bool _screenFilled{false};
    bool _showComments{true};

public:

    Curses(Screen& screen) : _screen(screen) {
        screen.registerOnNewDataAvailableListener([this](){
            if (!_screenFilled) {
                redraw();
            }
            else {
                drawMenu(true);
            }
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
            initColors();
        }

        LOG("Curses: " << _active);

        return _active;
    }

    void initColors() {
        // Positive colors
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
        init_pair(5, COLOR_BLUE, COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_RED, COLOR_BLACK);
        // Negative colors
        init_pair(8,  COLOR_BLACK, COLOR_WHITE);
        init_pair(9, COLOR_BLACK, COLOR_YELLOW);
        init_pair(10, COLOR_BLACK, COLOR_GREEN);
        init_pair(11, COLOR_BLACK, COLOR_CYAN);
        init_pair(12, COLOR_BLACK, COLOR_BLUE);
        init_pair(13, COLOR_BLACK, COLOR_MAGENTA);
        init_pair(14, COLOR_BLACK, COLOR_RED);
    }

    void setColor(uint8_t src, bool positive) {
        int index = positive ? (src % 7 + 1) : (src % 7 + 8);
        attron(COLOR_PAIR(index));
    }

    bool run() {

        if (!activate()) {
            return false;
        }

        while (true) {
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
                    redraw();
                    break;
                case KEY_UP:
                    _screen.scrollUp();
                    redraw();
                    break;
                case KEY_DOWN:
                    _screen.scrollDown();
                    redraw();
                    break;
                case 'q':
                case 'Q':
                    _active = false;
                    return true;
                    break;
                case 'c':
                case 'C':
                    toggleComments();
                    redraw();
                    break;
            }
        }

        return true;
    }

    void toggleComments() {
        _showComments = !_showComments;
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

    int drawMenu(bool standalone) {

        static const int kHorMargin = 2;
        static const int kVerMargin = 0;
        static const int kVerPadding = 2;
        static const int kRightPanelWidth = 20;

        int row = kVerMargin;
        int column = kHorMargin;

        if (standalone && !renderingAvailable()) {
            return 0;
        }

        for (int i = 0; i < _screen.getTabCnt(); i++) {

            auto tab = _screen.getTab(i);

            if (tab == nullptr) {
                return row + kVerPadding;
            }

            setColor(i, !tab->enabled);

            auto tabTitle = getTabTitle(*tab, i);

            mvprintw(row, column, "%s", tabTitle.c_str());

            if (standalone) {
                clrtoeol();
            }

            column += tabTitle.length() + kHorMargin;
        }

        if (standalone) {
            ::refresh();
            _drawing = false;
        }

        return row + kVerPadding;
    }

    bool drawLines(int startRow, int screenHeight) {

        _screen.prepareLines();
        int row = startRow;
        while (row < screenHeight) {

            auto line = _screen.nextLine();

            if (!line.isValid()) {
                LOG("Line not valid: " << line.id);
                return false;
            }

            setColor(line.src, true);
            row += printLine(row, line);

            if (!line.comment.empty() && _showComments) {
                row += printComment(row, line);
            }
        }

        LOG("Drawn " << row << "/" << screenHeight);

        return true;
    }

    bool renderingAvailable() {
        bool expected = false;
        return _drawing.compare_exchange_weak(expected, true);
    }
    
    void redraw() {

        if (!_active) {
            return;
        }

        if (!renderingAvailable()) {
            return;
        }

        int screenWidth, screenHeight;
        getmaxyx(stdscr, screenHeight, screenWidth);
       
        ::clear();
        auto menuHeight = drawMenu(false);
        _screenFilled = drawLines(menuHeight, screenHeight);

        ::refresh();
        _drawing = false;
    }

    int printLine(int row, const LogLine& line) {
        mvprintw(row, 0, "%6d [%d] %s", line.id, line.src, line.text.c_str());
        return getcury(stdscr) - row + 1;
    }

    int printComment(int row, const LogLine& line) {
        mvprintw(row, 0, "        |> %s", line.comment.c_str());
        return getcury(stdscr) - row + 1;
    }

    ~Curses() {
        endwin();
    }
};

