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
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    nlohmann::json json = nlohmann::json::parse(content);

    if(!json.contains("versions") || !json["versions"].is_object()) return vector<FFMPEG_VERSION>();

    nlohmann::json versions = json["versions"];

    vector<FFMPEG_VERSION> raw_list;

    for (auto& [key, value] : versions.items())
    {
        if(!value.contains(OS) || !value[OS].is_string()) continue;

        string url = value[OS];

        raw_list.push_back(FFMPEG_VERSION{version: key, url: url});
    }

    // versions.items() & raw_list is sorted, so to reverse start from the end
    vector<FFMPEG_VERSION> list(raw_list.size());

    for (size_t i = 0; i < raw_list.size(); i++)
    {
        list[raw_list.size() - i - 1] = raw_list[i];
    }
    
    
    return list;
}
