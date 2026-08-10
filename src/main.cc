#include "cli.hh"
#include "tui.hh"

#include <iostream>

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        CLI_EXIT_STATUS status = run_cli(argc, argv);

        if(status != CLI_EXIT_STATUS::OK) {
            std::cout << "Code exited with error: " << status << " (" << to_string(status) << ")" << std::endl;
        }

        return status;
    }

    return run_tui();
}
