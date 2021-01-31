#include "Curses.hpp"
#include "DataModel.hpp"
#include "LogReader.hpp"
#include "Options.hpp"

#include <string>

struct Configuration {
    DataModel data;
    std::vector<std::unique_ptr<LogReader>> readers;
};

bool initialize(Configuration* config, const Options::Options& options) {

    const auto noop = [](){};

    // Filters must be available before the input is read
    for (const auto& filter : options.filters) {
        config->data.addFilter(filter.name, filter.regex);
    }

    for (const auto& external : options.externals) {
        config->data.addExternal(external.name, external.command);
    }

    for (const auto& input : options.inputs) {
        config->readers.emplace_back(std::make_unique<LogReader>(input, noop, config->data.getAppender(input)));
    }

    return true;
}

int main(int argc, char* argv[]) {

    Options::Options options;
    Configuration configuration;
    Curses curses(configuration.data);

    if (!Options::parseOptions(&options, argc, argv)) {
        std::cout << Options::getHelp() << std::endl;
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
