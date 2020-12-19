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

//#define LOG(line) std::cerr << line << std::endl;
#define LOG(line) //

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
            while(read_line());
            _onStop();
        });
    }

    std::string get_line(const size_t lineNo) {
        std::lock_guard<std::mutex> g(_mtx);
        if (lineNo < _lines.size()) {
            return _lines[lineNo];
        }
    }

    bool read_line() {

        if (!_file.good()) {
            LOG("End of input reached.");
            return false;
        }

        std::string line;
        std::getline(_file, line);
        if (!line.empty()) {
            LOG("Read: \"" << line << "\", size: " << line.size());
            std::lock_guard<std::mutex> g(_mtx);
            _lines.emplace_back(line); // TODO :: Remove internal buffer
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
    std::vector<std::string> _lines;
    std::mutex               _mtx;
};

class Curses {

    bool _active = false;

public:

    bool activate() {
        initscr();
        noecho();
        refresh();
        _active = has_colors();

        if (_active) {
            start_color();
            init_pair(1, COLOR_RED, COLOR_YELLOW);
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
        }

        LOG("Curses: " << _active);

        return _active;
    }

    void print_line(int row, std::string line, int index) {

        if (_active) {
            attron(COLOR_PAIR(index));
            mvprintw(row, 0, "%4d %s", row, line.c_str());
            refresh();
            //LOG(row << ": " << line);

        }
    }

    ~Curses() {
        endwin();
    }
};


bool parse_options(int argc, char* argv[]) {
    return false;
}

class Screen {

    size_t row = 0;
public:

    Screen(Curses& curses) : _curses(curses) {}

    void printLn(const std::string& line) {
        ++row;
        _curses.print_line(row, line, row % 2 + 1);
    }

    Curses& _curses;
};

int main(int argc, char* argv[]) {

    Curses curses;
    curses.activate();
    Screen screen(curses);

    std::atomic<bool> isRunning(true);
    const auto stop = [&isRunning](){ isRunning = false; LOG("Stopped."); };
    const auto onReadLine = [&screen](auto line) { screen.printLn(line); };
        
    std::vector<std::unique_ptr<LogReader>> readers;
    readers.emplace_back(std::make_unique<LogReader>("in.txt", stop, onReadLine));

    int row = 0;
    std::string line;

    do {
        curses.print_line(row, line, row % 2 + 1);
        row++;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    } while (isRunning);

    return EXIT_SUCCESS;
}
