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

#include "Curses.hpp"
#include "Screen.hpp"
#include "LogReader.hpp"

namespace {
    std::function<void(int)> shutdownHandler;
    void signalHandler(int signal) { 
        if (shutdownHandler) {
            shutdownHandler(signal);
        }
    }
}

int main(int argc, char* argv[]) {

    Screen screen;
    Curses curses(screen);
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
        readers.emplace_back(std::make_unique<LogReader>(argv[i], noop, screen.getAppender(argv[i], i-1)));
    }

    curses.run();

/*
    std::unique_lock<std::mutex> g(cvMtx);
    cv.wait(g, [&isRunning] () { return !isRunning;});
*/
    for (auto& reader : readers) {
        reader->stop();
    }

    return EXIT_SUCCESS;
}
