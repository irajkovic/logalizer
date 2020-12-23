#include "Curses.hpp"
#include "Screen.hpp"
#include "LogReader.hpp"

struct Configuration {
    Screen screen;
    std::vector<std::unique_ptr<LogReader>> readers;
};

#include <cstring>

bool initialize(int argc, char* argv[], Configuration* config) {

    const auto noop = [](){};

    enum class Option { Input, Filter } option;
    int id = 0;

    for (int i=1; i<argc; i++) {

        LOG("Parsing option " << argv[i]);
        if (std::strcmp(argv[i], "-i") == 0) {
            option = Option::Input;
            LOG("Option Input");
        }
        else if (argv[i] == "-f") {
            option = Option::Filter;
            LOG("Option Filter");
        }
        else {
            switch (option) {
                case Option::Input:
                    LOG("Input value");
                    config->readers.emplace_back(std::make_unique<LogReader>(argv[i], noop, config->screen.getAppender(argv[i], id++)));
                    break;
                case Option::Filter:
                    LOG("Filter value");
                    break;
                default:
                    LOG("Option unrecognized");
                    return false;
            }
        }
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
