#include "Curses.hpp"
#include "Screen.hpp"
#include "LogReader.hpp"
#include "Options.hpp"

#include <string>

struct Configuration {
    Screen screen;
    std::vector<std::unique_ptr<LogReader>> readers;
};

bool initialize(Configuration* config, const Options& options) {

    const auto noop = [](){};

    int id = 0;

    for (const auto& input : options.inputs) {
        config->readers.emplace_back(std::make_unique<LogReader>(input, noop, config->screen.getAppender(input, id++)));
    }

    return true;
}

int main(int argc, char* argv[]) {

    Options options;
    Configuration configuration;
    Curses curses(configuration.screen);

    if (!parseOptions(&options, argc, argv)) {
        return EXIT_FAILURE;
    }

    if (!initialize(&configuration, options)) {
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
