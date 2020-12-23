#pragma once

#include <fstream>
#include <functional>
#include <future>

#include "Log.hpp"

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

