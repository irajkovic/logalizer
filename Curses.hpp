#pragma once

#include <atomic>
#include <curses.h>
#include <mutex>

#include "Screen.hpp"
#include "Log.hpp"

class Curses {

    static const size_t _menuHeight = 3u;
    std::atomic<bool> _active{false};
    std::vector<std::string> _tabs;
    std::mutex _mtx;
    Screen& _screen;

public:

    Curses(Screen& screen) : _screen(screen) {
        screen.registerOnNewDataAvailableListener([this](){
            refresh();
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
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(3, COLOR_YELLOW, COLOR_BLACK);
            init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(5, COLOR_CYAN, COLOR_BLACK);


            init_pair(6, COLOR_GREEN, COLOR_BLACK);
            init_pair(7, COLOR_YELLOW, COLOR_BLACK);
            init_pair(8, COLOR_MAGENTA, COLOR_BLACK);

            init_pair(9,  COLOR_BLACK, COLOR_WHITE);
            init_pair(10, COLOR_BLACK, COLOR_GREEN);
            init_pair(11, COLOR_BLACK, COLOR_YELLOW);
            init_pair(12, COLOR_BLACK, COLOR_MAGENTA);
            init_pair(13, COLOR_BLACK, COLOR_CYAN);

        }

        LOG("Curses: " << _active);

        return _active;
    }

    bool run() {

        if (!activate()) {
            return false;
        }

        while (true) {
            refresh();
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

    void drawMenu() {
        size_t column = 2u;
        std::lock_guard<std::mutex> g(_mtx);
        for (int i=0; i<_screen.getTabCnt(); i++) {
            auto tab = _screen.getTab(i);

            if (tab == nullptr) {
                return;
            }

            tab->enabled ? attron(COLOR_PAIR(i+9)) : attron(COLOR_PAIR(i+1));
            mvprintw(1, column, "[%d] %s", i, tab->name.c_str());
            column += tab->name.length() + 10u;
        }
    }
    
    void refresh() {

        int row, col;
        getmaxyx(stdscr, row, col);
       
        ::clear();
        drawMenu();

        _screen.prepareLines();
        for (int i=0; i<row; i++) {
            LogLine* line = _screen.nextLine();

            if (line == nullptr) {
                break;
            }

            printLine(i, line->text, line->id);
        }

        ::refresh();
    }

    void printLine(int row, std::string line, int index) {
        if (_active) {
            attron(COLOR_PAIR(index+1));
            std::lock_guard<std::mutex> g(_mtx);
            mvprintw(row + _menuHeight, 0, "[%i] %s", index, line.c_str());
        }
    }

    ~Curses() {
        endwin();
    }
};

