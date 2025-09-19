#include <memory> // for allocator, __shared_ptr_access, shared_ptr
#include <string> // for string, basic_string
#include <vector> // for vector
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iostream>
#include <thread>

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Radiobox, Horizontal, Menu, Renderer, Tab
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for MenuOption
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for Element, separator, hbox, operator|, border

#include "cli.hh"
#include "request.hh"
#include "environment.hh"
#include "ui_elements.hh"

using namespace ftxui;
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        return run_cli(argc, argv);
    }

    std::string FFMPEGVM_CURRENT_VERSION;

    {
        std::filesystem::path ffmpeg_vm_dir = get_ffmpeg_vm_dir();

        std::ifstream version_file(ffmpeg_vm_dir / "VERSION");
        if (version_file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(version_file)), std::istreambuf_iterator<char>());
            version_file.close();

            for (size_t i = 0; i < content.length(); i++)
            {
                if(content[i] == '\n') break;
                FFMPEGVM_CURRENT_VERSION.push_back(content[i]);
            }
        }
    }

    // Fetch versions
    std::cout << "Fetching versions..." << std::flush;

    // TODO: Fetch data
    std::vector<FFMPEG_VERSION> versions = get_ffmpeg_versions();

    // Remove the "Fetching versions..." line from console
    std::cout << "\r" << std::string(20, ' ') << "\r" << std::flush;

    std::vector<std::string> display_versions;

    for (const FFMPEG_VERSION &ver : versions)
    {
        if(!FFMPEGVM_CURRENT_VERSION.empty() && FFMPEGVM_CURRENT_VERSION == ver.version) display_versions.push_back(ver.version + " (Current)");
        else display_versions.push_back(ver.version);
    }

    ftxui::ScreenInteractive screen = ScreenInteractive::Fullscreen();

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

        FFMPEGVM_CURRENT_VERSION = version.version;

        std::filesystem::path downloaddir = get_ffmpeg_vm_dir();

        ftxui::ScreenInteractive download_screen = ScreenInteractive::Fullscreen();

        ftxui::Element display_text = text(center_text("Downloading ffmpeg " + version.version + "..."));
        ftxui::Element display_slider = text(generate_slider(0));

        ftxui::Component download_renderer = Renderer(Container::Horizontal({}), [&]
        {
            ftxui::Element alert_window = vbox({
                display_text | borderEmpty,
                display_slider | borderEmpty
            });

            alert_window = alert_window | borderEmpty | border | size(WIDTH, LESS_THAN, 80) |
                    size(HEIGHT, LESS_THAN, 20) | center;
            return alert_window;
        });

        std::thread download_thread([&]
        {
            remove_env();

            setup_env(version.version);

            const std::string fdata = download_file(version.url, &display_slider, &download_screen);

            display_text = text(center_text("Extracting files..."));
            display_slider = text(generate_slider(0.0f));
            download_screen.PostEvent(ftxui::Event::Custom);

            extract(fdata, downloaddir, &display_slider, &download_screen);

            display_text = text(center_text("Done!"));
            display_slider = text(generate_slider(1.0f));
            download_screen.PostEvent(ftxui::Event::Custom);

            std::this_thread::sleep_for(std::chrono::seconds(1));

            display_versions.clear();

            // Update current version
            for (const FFMPEG_VERSION &ver : versions)
            {
                if(!FFMPEGVM_CURRENT_VERSION.empty() && FFMPEGVM_CURRENT_VERSION == ver.version) display_versions.push_back(ver.version + " (Current)");
                else display_versions.push_back(ver.version);
            }

            download_screen.Exit();
        });

        download_thread.detach();

        download_screen.Loop(download_renderer);
    };

    ftxui::Component version_selector_component = Menu(&display_versions, &version_selected, version_selector_options);

    ftxui::Component list_container_component = Container::Tab(
        {version_selector_component | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
         Button("Let's uninstall", [&]
                { remove_env(); FFMPEGVM_CURRENT_VERSION = ""; display_versions.clear(); for (const FFMPEG_VERSION &ver : versions) display_versions.push_back(ver.version); display_alert(text("Uninstall complete!"), std::chrono::seconds(2)); }, ButtonOption::Ascii()),
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
            return window(text("ffmpeg-version-manager v0.1.5"),
                          hbox({
                              menus_component->Render() | borderEmpty | size(WIDTH, EQUAL, 15),
                              separator(),
                              list_container_component->Render() | borderEmpty,
                          }));
        });

    screen.Loop(renderer);

    return 0;
}
