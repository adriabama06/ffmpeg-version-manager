
// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <memory> // for allocator, __shared_ptr_access, shared_ptr
#include <string> // for string, basic_string
#include <vector> // for vector

#include "ftxui/component/captured_mouse.hpp"     // for ftxui
#include "ftxui/component/component.hpp"          // for Radiobox, Horizontal, Menu, Renderer, Tab
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/dom/elements.hpp"                 // for Element, separator, hbox, operator|, border

#include "request.hh"

using namespace ftxui;

int main()
{
    // Fetch versions
    std::cout << "Fetching versions..." << std::flush;
    
    // TODO: Fetch data
    
    // Remove the "Fetching versions..." line from console
    std::cout << "\r" << std::string(20, ' ') << "\r" << std::flush;
    
    for (const FFMPEG_VERSION &ver : get_ffmpeg_versions())
    {
        std::cout << ver.version << " -> " << ver.url << std::endl;
    }
    
    auto screen = ScreenInteractive::TerminalOutput();

    std::vector<std::string> menus{
        "Install",
        "Uninstall",
        "Exit",
    };

    int menu_selected = 0;
    auto tab_menu = Menu(&menus, &menu_selected);

    std::vector<std::string> tab_1_entries{
        "Forest",
        "Water",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I don't know",
        "I know"};
    int tab_1_selected = 0;

    std::vector<std::string> tab_2_entries{
        "Hello",
        "Hi",
        "Hay",
    };
    int tab_2_selected = 0;

    std::vector<std::string> tab_3_entries{
        "Table",
        "Nothing",
        "Is",
        "Empty",
    };

    int tab_3_selected = 0;

    auto tab_container = Container::Tab(
        {
            Radiobox(&tab_1_entries, &tab_1_selected) | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
            Button("Let's uninstall", [] { std::cout << "Uninstall pressed" << std:: endl; }, ButtonOption::Ascii()),
            Button("Ok, exit", screen.ExitLoopClosure(), ButtonOption::Ascii())
        },
        &menu_selected);

    auto container = Container::Horizontal({
        tab_menu,
        tab_container,
    });

    auto renderer = Renderer(
        container,
        [&]
        {
            return hbox({
                       tab_menu->Render(),
                       separator(),
                       tab_container->Render(),
                   }) |
                   border;
        });

    screen.Loop(renderer);
}
