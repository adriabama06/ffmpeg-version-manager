#include <memory> // for allocator, __shared_ptr_access, shared_ptr
#include <string> // for string, basic_string
#include <vector> // for vector
#include <filesystem>

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Radiobox, Horizontal, Menu, Renderer, Tab
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for MenuOption
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for Element, separator, hbox, operator|, border

#include "request.hh"
#include "environment.hh"

using namespace ftxui;

int main()
{
    // Fetch versions
    std::cout << "Fetching versions..." << std::flush;

    // TODO: Fetch data
    std::vector<FFMPEG_VERSION> versions = get_ffmpeg_versions();

    // Remove the "Fetching versions..." line from console
    std::cout << "\r" << std::string(20, ' ') << "\r" << std::flush;

    std::vector<std::string> display_versions;

    for (const FFMPEG_VERSION &ver : versions)
    {
        display_versions.push_back(ver.version);
    }

    ftxui::ScreenInteractive screen = ScreenInteractive::TerminalOutput();

    std::vector<std::string> menus{
        "Install",
        "Uninstall",
        "Exit",
    };

    int menu_selected = 0;
    ftxui::Component menus_component = Menu(&menus, &menu_selected);

    int version_selected = 0;

    MenuOption version_selector_options;
    version_selector_options.on_enter = [&]
    {
        FFMPEG_VERSION version = versions[version_selected];

        remove_env();

        setup_env();

        std::filesystem::path downloaddir = get_ffmpeg_vm_dir();

        const std::string fdata = download_file(version.url);

        extract(fdata, downloaddir);
    };

    ftxui::Component version_selector_component = Menu(&display_versions, &version_selected, version_selector_options);

    ftxui::Component list_container_component = Container::Tab(
        {version_selector_component | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
         Button("Let's uninstall", []
                { remove_env(); }, ButtonOption::Ascii()),
         Button("Ok, exit", screen.ExitLoopClosure(), ButtonOption::Ascii())},
        &menu_selected);

    ftxui::Component container = Container::Horizontal({
        menus_component,
        list_container_component,
    });

    // Add a separator and a border
    ftxui::Component renderer = Renderer(
        container,
        [&]
        {
            return hbox({
                       menus_component->Render(),
                       separator(),
                       list_container_component->Render(),
                   }) |
                   border;
        });

    screen.Loop(renderer);

    return 0;
}
