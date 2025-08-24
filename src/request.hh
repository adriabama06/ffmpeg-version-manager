#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

#ifndef REQUEST_H
#define REQUEST_H

typedef struct FFMPEG_VERSION_S {
    std::string version;
    std::string url;
} FFMPEG_VERSION;

std::vector<FFMPEG_VERSION> get_ffmpeg_versions();
std::string download_file(std::string url);
int extract(const std::string &filedata, const std::filesystem::path &destination_dir);

#endif // REQUEST_H
