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

int main(int argc, char* argv[]) {

    Screen screen;
    Curses curses(screen);
    const auto noop = [](){};

    std::vector<std::unique_ptr<LogReader>> readers;

    for (int i=1; i<argc; i++) {
        LOG("Reading file " << argv[i]);
        readers.emplace_back(std::make_unique<LogReader>(argv[i], noop, screen.getAppender(argv[i], i-1)));
    }

    if (!curses.run()) {
        return EXIT_FAILURE;
    }

    for (auto& reader : readers) {
        reader->stop();
    }

    return EXIT_SUCCESS;
}
