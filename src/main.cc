#include "cli.hh"
#include "tui.hh"

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        return run_cli(argc, argv);
    }

    return run_tui();
}
