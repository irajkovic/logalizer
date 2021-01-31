#pragma once

#include <atomic>
#include <curses.h>
#include <mutex>

#include "IDataModel.hpp"
#include "Log.hpp"

class Curses {

    std::atomic<bool> _active{false};
    std::vector<std::string> _tabs;
    IDataModel& _data;
    std::atomic<bool> _drawing{false};
    std::atomic<bool> _screenFilled{false};
    bool _showComments{true};

    WINDOW* _winMenu{nullptr};
    WINDOW* _winLines{nullptr};

    static const int kMenuHeight = 3;
    static const int kMenuWidth = 0; // Full width

public:

    Curses(IDataModel& data)
        :_data(data) {

        _data.registerOnNewDataAvailableListener([this](){
            if (!_screenFilled) {
                redraw();
            }
            else {
                drawMenu();
            }
        });
    }

    ~Curses() {
        ::delwin(_winMenu);
        ::delwin(_winLines);
        endwin();
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

        _winMenu = ::newwin(kMenuHeight, kMenuWidth, 0, 0);
        _winLines = ::newwin(0, 0, kMenuHeight, 0);

        LOG("Curses: " << _active);

        return _active && _winMenu && _winLines;
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

    void setColor(WINDOW* window, uint8_t src, bool positive) {
        int index = positive ? (src % 7 + 1) : (src % 7 + 8);
        ::wattron(window, COLOR_PAIR(index));
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
                    _data.toggleTab(ch - '0');
                    break;
                case KEY_UP:
                    _data.scrollUp();
                    break;
                case KEY_DOWN:
                    _data.scrollDown();
                    break;
                case 'q':
                case 'Q':
                    _active = false;
                    return true;
                    break;
                case 'c':
                case 'C':
                    toggleComments();
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

    int drawMenu() {

        static const int kHorMargin = 2;
        static const int kVerMargin = 0;
        static const int kVerPadding = 2;
        static const int kRightPanelWidth = 20;

        int row = kVerMargin;
        int column = kHorMargin;

        ::wclear(_winMenu);

        for (int i = 0; i < _data.getTabCnt(); i++) {

            auto tab = _data.getTab(i);

            if (!tab) {
                return row + kVerPadding;
            }

            setColor(_winMenu, i, !tab.enabled);

            auto tabTitle = getTabTitle(tab, i);

            ::mvwprintw(_winMenu, row, column, "%s", tabTitle.c_str());

            column += tabTitle.length() + kHorMargin;
        }

        wrefresh(_winMenu);
        return row + kVerPadding;
    }

    bool drawLines(int screenHeight) {

        _data.prepareLines();
        int row = 0;
        while (row < screenHeight) {

            auto line = _data.nextLine();

            if (!line.isValid()) {
                LOG("Line not valid: " << line.id);
                return false;
            }

            setColor(_winLines, line.src, true);
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
        getmaxyx(_winLines, screenHeight, screenWidth);

        LOG("Dimensions: " << screenWidth << "x" << screenHeight);
       
        ::wclear(_winLines);
        drawMenu();
        _screenFilled = drawLines(screenHeight);

        ::wrefresh(_winLines);
        _drawing = false;
    }

    int printLine(int row, const LogLine& line) {
        ::mvwprintw(_winLines, row, 0, "%6d [%d] %s", line.id, line.src, line.text.c_str());
        return getcury(_winLines) - row + 1;
    }

    int printComment(int row, const LogLine& line) {
        ::mvwprintw(_winLines, row, 0, "        |> %s", line.comment.c_str());
        return getcury(_winLines) - row + 1;
    }
};

