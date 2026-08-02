#ifndef CLI_H
#define CLI_H

enum CLI_EXIT_STATUS {
    OK = 0,
    UNKNONW_OPTION,
    NO_ARGUMENT,
    NO_VERSIONS_TO_INSTALL,
    ERROR_DOWNLOADING,
    ERROR_EXTRACTING,
    ALREADY_INSTALLED
};

CLI_EXIT_STATUS run_cli(int argc, char *argv[]);

#endif // CLI_H
