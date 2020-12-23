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

struct Configuration {
    Screen screen;
    std::vector<std::unique_ptr<LogReader>> readers;
};

bool initialize(int argc, char* argv[], Configuration* config) {

    const auto noop = [](){};

    for (int i=1; i<argc; i++) {
        LOG("Reading file " << argv[i]);
        config->readers.emplace_back(std::make_unique<LogReader>(argv[i], noop, config->screen.getAppender(argv[i], i-1)));
    }

    return true;
}

int main(int argc, char* argv[]) {

    Configuration configuration;
    Curses curses(configuration.screen);

    if (!initialize(argc, argv, &configuration)) {
        return EXIT_FAILURE;
    }

    if (!curses.run()) {
        return EXIT_FAILURE;
    }

    for (auto& reader : configuration.readers) {
        reader->stop();
    }

    return EXIT_SUCCESS;
}
