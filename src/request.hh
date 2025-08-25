#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include "ftxui/component/component.hpp"       // for Element

#ifndef REQUEST_H
#define REQUEST_H

typedef struct FFMPEG_VERSION_S {
    std::string version;
    std::string url;
} FFMPEG_VERSION;

std::vector<FFMPEG_VERSION> get_ffmpeg_versions();
std::string download_file(std::string url, ftxui::Element* display_slider, ftxui::ScreenInteractive* screen);
int extract(const std::string &filedata, const std::filesystem::path &destination_dir, ftxui::Element* display_slider, ftxui::ScreenInteractive* screen);

#endif // REQUEST_H
