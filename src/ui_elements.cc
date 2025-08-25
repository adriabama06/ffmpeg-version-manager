#include "ui_elements.hh"

#include <thread>

#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/component/component.hpp"       // for Make, Renderer

using namespace ftxui;


void display_alert(ftxui::Element content, const std::chrono::seconds t)
{
    ftxui::ScreenInteractive alert_screen = ScreenInteractive::Fullscreen();

        ftxui::Component alert_renderer = Renderer(Container::Horizontal({}), [&]
        {
            ftxui::Element alert_window = vbox({
                content | borderEmpty
            });

            alert_window = alert_window | borderEmpty | border | size(WIDTH, LESS_THAN, 80) |
                    size(HEIGHT, LESS_THAN, 20) | center;
            return alert_window;
        });

        std::thread alert_thread([&]
        {
            std::this_thread::sleep_for(t);

            alert_screen.Exit();
        });

        alert_thread.detach();

        alert_screen.Loop(alert_renderer);
}

std::string generate_slider(float percent)
{
    int i;
    int max = 50;
    int middle = max * percent;
    
    std::string sl = "[";

    for (i = 0; i < middle; i++)
    {
        sl += '#';
    }
    for (i = middle; i < max; i++)
    {
        sl += ' ';
    }

    sl += ']';

    return sl;
}

std::string center_text(std::string str)
{
    int i;
    int max = 50;
    int middle = max / 2;

    int len = str.length();

    std::string cent = "";

    for (i = 0; i < middle - (len / 2); i++)
    {
        cent += ' ';
    }

    cent += str;

    for (i = middle + (len / 2); i < max; i++)
    {
        cent += ' ';
    }

    return cent;
}