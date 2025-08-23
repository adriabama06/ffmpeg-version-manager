#include "request.hh"

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

using namespace std;

#ifdef _WIN32
#define OS "windows"
#else
#define OS "linux"
#endif

vector<FFMPEG_VERSION> get_ffmpeg_versions()
{
    ifstream file("ffmpeg-list.json");

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);
    
    std::string content(size, '\0');
    file.read(&content[0], size); 

    nlohmann::json json = nlohmann::json::parse(content);

    if(!json.contains("versions") || !json["versions"].is_object()) return vector<FFMPEG_VERSION>();

    nlohmann::json versions = json["versions"];

    vector<FFMPEG_VERSION> list = vector<FFMPEG_VERSION>(versions.size());

    // versions.items() is sorted, so to reverse start from the end
    size_t i = versions.size() - 1;
    for (auto& [key, value] : versions.items())
    {
        if(!value.contains(OS) || !value[OS].is_string())
        {
            list[i--] = FFMPEG_VERSION{version: "", url: ""};
            continue;
        }

        string url = value[OS];

        list[i--] = FFMPEG_VERSION{version: key, url: url};
    }

    return list;
}
