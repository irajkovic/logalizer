#pragma once

#include <cstring>
#include <vector>
#include <string>

#include "Log.hpp"

struct Filter {
    std::string name;
    std::string regex;
};

struct Options {
    std::vector<std::string> inputs;
    std::vector<Filter> filters;
};


Filter parseFilter(const std::string& raw) {
    return {}; // TODO
}

bool parseOptions(Options* options, int argc, char* argv[]) {

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
                    options->inputs.emplace_back(argv[i]);
                    break;
                case Option::Filter:
                    options->filters.emplace_back(parseFilter(argv[i]));
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

