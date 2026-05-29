#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include "ftxui/component/component.hpp"       // for Element

#ifndef REQUEST_H
#define REQUEST_H

#define FFMPEGVM_URL "https://raw.githubusercontent.com/adriabama06/FFmpeg-Builds-Archive/refs/heads/main/ffmpeg-list.json"

typedef struct FFMPEG_VERSION_S {
    std::string version;
    std::string url;
} FFMPEG_VERSION;

inline std::ostream& operator<<(std::ostream& ostr, const FFMPEG_VERSION& ver)
{
    ostr << ver.version << ": " << ver.url;

    return ostr;
}

inline bool operator<(const FFMPEG_VERSION& a, const FFMPEG_VERSION& b)
{
    return a.version < b.version;
}

std::vector<FFMPEG_VERSION> get_ffmpeg_versions();
std::string display_download_file(std::string url, ftxui::Element* display_slider, ftxui::ScreenInteractive* screen);
int extract(const std::string &filedata, const std::filesystem::path &destination_dir, ftxui::Element* display_slider, ftxui::ScreenInteractive* screen);

#endif // REQUEST_H
