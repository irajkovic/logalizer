#pragma once

#include <cstring>
#include <vector>
#include <string>
#include <optional>

#include "Log.hpp"

struct Filter {
    std::string name;
    std::string regex;
};

struct Options {
    std::vector<std::string> inputs;
    std::vector<Filter> filters;
};


std::optional<Filter> parseFilter(const std::string& raw) {
    // Expected format "name:regex"
    size_t separatorInd = raw.find(":");

    if (separatorInd == std::string::npos) {
        return std::nullopt;
    }

    if (separatorInd >= raw.length()) {
        return std::nullopt;
    }

    Filter filter;
    filter.name = raw.substr(0, separatorInd);
    filter.regex = raw.substr(separatorInd + 1, std::string::npos);
    return filter;
}

bool parseOptions(Options* options, int argc, char* argv[]) {

    enum class Option { Input, Filter } option;
    int id = 0;

    for (int i=1; i<argc; i++) {

        LOG("Parsing option " << argv[i]);
        if (std::strcmp(argv[i], "-i") == 0) {
            option = Option::Input;
        }
        else if (std::strcmp(argv[i], "-f") == 0) {
            option = Option::Filter;
        }
        else {
            switch (option) {
                case Option::Input:
                    options->inputs.emplace_back(argv[i]);
                    break;
                case Option::Filter:
                    if (auto filter = parseFilter(argv[i])) {
                        LOG("Parsed filter: " << filter->name << ", " << filter->regex);
                        options->filters.emplace_back(*filter);
                    }
                    else {
                        LOG("Cannot parse filter: " << argv[i]);
                        return false;
                    }
                    break;
                default:
                    LOG("Option unrecognized: " << argv[i]);
                    return false;
            }
        }
    }

    return true;
}

