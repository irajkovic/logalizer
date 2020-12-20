#include <curses.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <functional>
#include <future>
#include <csignal>
#include <chrono>

#define LOG(line) std::cerr << __FUNCTION__ << ": " <<  line << std::endl;
//#define LOG(line)

class LogReader {

public:

    using OnStop = std::function<void()>;
    using OnReadLine = std::function<void(std::string)>;

    LogReader(std::string filename, OnStop onStop, OnReadLine onReadLine)
        : _filename(filename)
        , _onStop(onStop)
        , _onReadLine(onReadLine) {
        _file.open(filename);
        start();
    }

    void start() {
        _worker = std::async(std::launch::async, [this] () {
            _isOn = true;
            while(_isOn && read_line());
            _onStop();
        });
    }

    void stop() {
        _isOn = false;
    }

    bool read_line() {

        if (!_file.good()) {
            LOG("End of input reached.");
            return false;
        }

        std::string line;
        std::getline(_file, line);
        if (!line.empty()) {
            _onReadLine(line);
        }

        return true;
    }

private:

    std::ifstream            _file;
    std::string              _filename;
    OnStop                   _onStop;
    OnReadLine               _onReadLine;
    std::future<void>        _worker;
    std::atomic<bool>        _isOn;
};

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


bool parse_options(int argc, char* argv[]) {
    return false;
}

struct LogLine {
    std::chrono::time_point<std::chrono::steady_clock> time;
    std::string text;
    size_t sourceId;
};

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

namespace {
    std::function<void(int)> shutdownHandler;
    void signalHandler(int signal) { 
        if (shutdownHandler) {
            shutdownHandler(signal);
        }
    }
}

int main(int argc, char* argv[]) {

    Curses curses;
    curses.activate();
    Screen screen(curses);
    std::mutex cvMtx;
    std::condition_variable cv;
    bool isRunning(true);

    const auto stop = [&cv, &cvMtx, &isRunning] () { 
        { std::lock_guard<std::mutex> g(cvMtx); isRunning = false; } 
        cv.notify_one(); 
        LOG("Stopped."); 
    };
    const auto noop = [](){};

    shutdownHandler = [&stop](int /*signal*/){ stop(); };
    std::signal(SIGINT, signalHandler);
        
    std::vector<std::unique_ptr<LogReader>> readers;

    for (int i=1; i<argc; i++) {
        LOG("Reading file " << argv[i]);
        readers.emplace_back(std::make_unique<LogReader>(argv[i], noop, screen.getAppender(argv[i], i)));
    }

    while (true) {
        t screen.redraw();
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                screen.scrollUp();
                break;
            case KEY_DOWN:
                screen.scrollDown();
                break;
            case 'q':
            case 'Q':
                stop();
                break;
        }

        std::unique_lock<std::mutex> g(cvMtx);
        if (!isRunning) {
            break;
        }
        LOG("Read user input: " << std::hex << ch << " UP " << KEY_UP << " DOWN " << KEY_DOWN);
    }

/*
    std::unique_lock<std::mutex> g(cvMtx);
    cv.wait(g, [&isRunning] () { return !isRunning;});
*/
    for (auto& reader : readers) {
        reader->stop();
    }

    return EXIT_SUCCESS;
}
