#include <vector>
#include <string>

#ifndef REQUEST_H
#define REQUEST_H

typedef struct FFMPEG_VERSION_S {
    std::string version;
    std::string url;
} FFMPEG_VERSION;

std::vector<FFMPEG_VERSION> get_ffmpeg_versions();

#endif // REQUEST_H