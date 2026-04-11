#include <chrono>
#include <string>

#include "ftxui/dom/elements.hpp"                 // for Element

#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

void display_alert(ftxui::Element content, const std::chrono::seconds t);
std::string generate_slider(float percent);
std::string center_text(std::string str);

#endif // UI_ELEMENTS_H