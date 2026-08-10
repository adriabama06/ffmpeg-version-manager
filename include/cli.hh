#ifndef CLI_H
#define CLI_H

#include "string"

enum CLI_EXIT_STATUS {
    OK = 0,
    UNKNONW_OPTION,
    NO_ARGUMENT,
    NO_VERSIONS_TO_INSTALL,
    ERROR_DOWNLOADING,
    ERROR_EXTRACTING,
    ALREADY_INSTALLED
};

inline std::string to_string(CLI_EXIT_STATUS status) {
    switch (status) {
        case CLI_EXIT_STATUS::OK: return "OK";
        case CLI_EXIT_STATUS::UNKNONW_OPTION: return "UNKNONW_OPTION";
        case CLI_EXIT_STATUS::NO_ARGUMENT: return "NO_ARGUMENT";
        case CLI_EXIT_STATUS::NO_VERSIONS_TO_INSTALL: return "NO_VERSIONS_TO_INSTALL";
        case CLI_EXIT_STATUS::ERROR_DOWNLOADING: return "ERROR_DOWNLOADING";
        case CLI_EXIT_STATUS::ERROR_EXTRACTING: return "ERROR_EXTRACTING";
        case CLI_EXIT_STATUS::ALREADY_INSTALLED: return "ALREADY_INSTALLED";
    }
    return "Unknown";
}

CLI_EXIT_STATUS run_cli(int argc, char *argv[]);

#endif // CLI_H
