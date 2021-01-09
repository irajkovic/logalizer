#pragma once

#include <cstring>
#include <vector>
#include <string>
#include <optional>

#include "Log.hpp"

namespace {

    const char* help =
        "\nLogalizer\n\n"
        "Log Analysis tool.\n\n"
        "Options:\n"
        "  -i <file>\n"
        "     Input file to be read.\n"
        "  -f <name:regex>\n"
        "     Defines a regular expression with the given name.\n"
        "     Matching lines will be marked with a color and\n"
        "     their visibility can be toggled.\n"
        "  -e <name:command>\n"
        "     Defines a command to be executed whenever a line\n"
        "     with matches a filter with the same name.\n"
        "     The whole line will be passed to the given command\n"
        "     as its first argument and anything outputed to the\n"
        "     standard output by the command will be recorded and\n"
        "     displayed as a comment.\n"
        "  -h Prints this help.\n\n"
        "Example:\n\n"
		"  log-analyzer -i  <(journalctl) -f 'KERNEL:.*kernel.*' 'SYSTEMD:.*systemd.*' 2> err.txt\n\n"
		"     Reads the contents of journalctl and marks all lines containing \"kernel\" and \"systemd\".\n";
}

namespace Options {

struct Filter {
    std::string name;
    std::string regex;
};

struct External {
    std::string name;
    std::string command;
};

struct Options {
    std::vector<std::string> inputs;
    std::vector<Filter> filters;
    std::vector<External> externals;
};

const char* getHelp() {
    return help;
}

std::pair<std::string, std::string> parseNameValuePair(bool *success, const std::string& raw) {
    // Expected format "name:value"
    size_t separatorInd = raw.find(":");

    if (separatorInd == std::string::npos) {
        return {};
    }

    if (separatorInd >= raw.length()) {
        return {};
    }

    *success = true;
    return std::make_pair(raw.substr(0, separatorInd), raw.substr(separatorInd + 1, std::string::npos));
}

template <typename T>
void parse(bool *success, std::vector<T>* out, const std::string& raw) {
    auto parsed = parseNameValuePair(success, raw);
    if (*success) {
        out->emplace_back(T{parsed.first, parsed.second});
    }
}

void parse(bool *success, std::vector<std::string>* out, const std::string& raw) {
    out->emplace_back(raw);
    *success = true;
}

bool parseOptions(Options* options, int argc, char* argv[]) {

    enum class Option { Input, Filter, External } option;
    int id = 0;

    for (int i=1; i<argc; i++) {

        if (std::strcmp(argv[i], "-h") == 0) {
            return false;
        }
        else if (std::strcmp(argv[i], "-i") == 0) {
            option = Option::Input;
        }
        else if (std::strcmp(argv[i], "-f") == 0) {
            option = Option::Filter;
        }
        else if (std::strcmp(argv[i], "-e") == 0) {
            option = Option::External;
        }
        else {
            bool success = false;
            switch (option) {
                case Option::Input:
                    parse(&success, &options->inputs, argv[i]);
                    break;
                case Option::Filter:
                    parse(&success, &options->filters,  argv[i]);
                    break;
                case Option::External:
                    parse(&success, &options->externals, argv[i]);
                    break;
            }

            if (!success) {
                LOG("Failed to parse option: " << argv[i]);
                return false;
            }
        }
    }

    return true;
}

} // namespace Options
