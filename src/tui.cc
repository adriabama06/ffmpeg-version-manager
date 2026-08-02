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

#include "tui.hh"
#include "request.hh"
#include "environment.hh"
#include "ui_elements.hh"
#include "ffmpeg-vm.hh"

using namespace ftxui;
namespace fs = std::filesystem;

void generate_display_versions(std::vector<std::string>& result, const std::vector<FFMPEG_VERSION>& versions, const std::string& current_version);
void uninstall_workflow(std::vector<std::string>& display_versions, const std::vector<FFMPEG_VERSION>& versions, std::string& current_version);
void download_workflow(int version_selected, const std::vector<FFMPEG_VERSION>& versions, std::string& current_version, std::vector<std::string>& display_versions);

int run_tui()
{
    std::string FFMPEGVM_CURRENT_VERSION = get_current_version();

    // Fetch versions
    std::vector<FFMPEG_VERSION> versions;
    {
        std::string fetch_message = "Fetching versions from '" + std::string(getenv("FFMPEGVM_URL") != NULL ? getenv("FFMPEGVM_URL") : FFMPEGVM_URL) + "'...";
        std::cout << fetch_message << std::flush;

        versions = get_ffmpeg_versions();

        // Remove the "Fetching versions..." line from console
        std::cout << "\r" << std::string(fetch_message.length(), ' ') << "\r" << std::flush;
    }

    std::vector<std::string> display_versions;
    generate_display_versions(display_versions, versions, FFMPEGVM_CURRENT_VERSION);

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
    version_selector_options.on_enter = [&] { download_workflow(version_selected, versions, FFMPEGVM_CURRENT_VERSION, display_versions); };

    ftxui::Component version_selector_component = Menu(&display_versions, &version_selected, version_selector_options);

    ftxui::Component list_container_component = Container::Tab(
        {
            version_selector_component | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
            Button("Let's uninstall", [&] { uninstall_workflow(display_versions, versions, FFMPEGVM_CURRENT_VERSION); }, ButtonOption::Ascii()),
            Button("Ok, exit", screen.ExitLoopClosure(), ButtonOption::Ascii())
        },
        &menu_selected
    );

    ftxui::Component container = Container::Horizontal({
        menus_component,
        list_container_component,
    });

    // Add a separator and a border
    ftxui::Component renderer = Renderer(
        container,
        [&]
        {
            return window(text(std::string(PROGRAM_NAME) + " v" + std::string(PROGRAM_VERSION)),
                        hbox({
                            menus_component->Render() | borderEmpty | size(WIDTH, EQUAL, 15),
                            separator(),
                            list_container_component->Render() | borderEmpty,
                        })
                    );
        });

    screen.Loop(renderer);

    return 0;
}

void generate_display_versions(std::vector<std::string>& result, const std::vector<FFMPEG_VERSION>& versions, const std::string& current_version)
{
    result.clear();

    for (const FFMPEG_VERSION& ver : versions)
    {
        if(!current_version.empty() && current_version == ver.version) result.push_back(ver.version + " (Current)");
        else result.push_back(ver.version);
    }
}

void uninstall_workflow(std::vector<std::string>& display_versions, const std::vector<FFMPEG_VERSION>& versions, std::string& current_version)
{
    remove_env();
    current_version.clear();
    generate_display_versions(display_versions, versions, current_version);
    display_alert(text("Uninstall complete!"), std::chrono::seconds(2));
}

void download_workflow(int version_selected, const std::vector<FFMPEG_VERSION>& versions, std::string& current_version, std::vector<std::string>& display_versions)
{
    FFMPEG_VERSION version = versions[static_cast<size_t>(version_selected)];

    current_version = version.version;

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

        alert_window = alert_window | borderEmpty | border | size(WIDTH, LESS_THAN, 80) | size(HEIGHT, LESS_THAN, 20) | center;
        return alert_window;
    });

    std::thread download_thread([&]
    {
        remove_env();

        setup_env(version.version);

        const std::string fdata = display_download_file(version.url, &display_slider, &download_screen);

        display_text = text(center_text("Extracting files..."));
        display_slider = text(generate_slider(0));
        download_screen.PostEvent(ftxui::Event::Custom);

        display_extract(fdata, downloaddir, &display_slider, &download_screen);

        display_text = text(center_text("Done!"));
        display_slider = text(generate_slider(1.0f));
        download_screen.PostEvent(ftxui::Event::Custom);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Update current version
        generate_display_versions(display_versions, versions, current_version);

        download_screen.Exit();
    });

    download_thread.detach();

    download_screen.Loop(download_renderer);
}
