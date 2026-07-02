#include "request.hh"
#include "ui_elements.hh"
#include "curl_tools.hh"

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include <nlohmann/json.hpp>

#include "compress.hh"

#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive

using namespace std;
namespace fs = std::filesystem;

#ifdef _WIN32
#define OS "windows"
#else
#define OS "linux"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define ARCH "x64"
#pragma message("Compiling for x64 (x86_64 / AMD64)")
#elif defined(__aarch64__) || defined(_M_ARM64)
#pragma message("Compiling for ARM64 (AArch64)")
#define ARCH "arm64"
#else
#error "Unknown architecture — build cancelled!"
#endif

typedef struct PROGRESSDATA_S {
    ftxui::Element* display_slider;
    ftxui::ScreenInteractive* screen;
} PROGRESSDATA;

// Callback function to write curl response to a string
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

// https://github.com/dryark/minibrew_deploy/blob/main/curlprog.m#L31
float last_progress = 0;

static int DownloadCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    PROGRESSDATA* data = reinterpret_cast<PROGRESSDATA*>(clientp);

    ftxui::Element* display_slider = data->display_slider;
    ftxui::ScreenInteractive* screen = data->screen;

    if (dltotal > 0)
    {
        float progress = static_cast<float>(dlnow) / static_cast<float>(dltotal);

        // Reduce the amount of updates/s
        if (progress - last_progress > 0.05)
        {
            *display_slider = ftxui::text(generate_slider(progress > 0.95 ? 1.0f : progress));

            screen->PostEvent(ftxui::Event::Custom);

            last_progress = progress;
        }
    }

    return 0; // Return 0 to continue the download
}

vector<FFMPEG_VERSION> get_ffmpeg_versions()
{
    const char* custom_url = getenv("FFMPEGVM_URL");
    string response;
    int status = quick_curl_request(custom_url != NULL ? custom_url : FFMPEGVM_URL, &response);

    // Error on request
    if (status != 0 || response.empty())
        return {};

    nlohmann::json json = nlohmann::json::parse(response);

    if (!json.contains("versions") || !json["versions"].is_object())
        return {};

    nlohmann::json versions = json["versions"];

    vector<FFMPEG_VERSION> raw_list;

    for (auto &[key, value] : versions.items())
    {
        if (!value.contains(OS) || !value[OS].contains(ARCH) || !value[OS][ARCH].is_string())
            continue;

        string url = value[OS][ARCH];

        // No version available for this OS and ARCH
        if(url.empty())
            continue;

        raw_list.push_back(FFMPEG_VERSION{.version = key, .url = url});
    }

    // versions.items() & raw_list is sorted, so to reverse start from the end
    vector<FFMPEG_VERSION> list(raw_list.size());

    for (size_t i = 0; i < raw_list.size(); i++)
    {
        list[raw_list.size() - i - 1] = raw_list[i];
    }

    return list;
}

string display_download_file(string url, ftxui::Element* display_slider, ftxui::ScreenInteractive* screen)
{
    CURL *curl = init_curl_request(url);
    string response;

    assert(display_slider != NULL && screen != NULL); // Use this function with a display

    PROGRESSDATA data;

    data.display_slider = display_slider;
    data.screen = screen;

    set_progress_callback_curl(curl, &data, DownloadCallback);

    CURLcode res = launch_curl_request_result(curl, &response);

    if(is_curl_cert_error(res))
    {
        response.clear();

        set_unsecure_curl(curl);
        res = launch_curl_request_result(curl, &response);
    }

    // After downloading reset window to 0
    last_progress = 0;

    destroy_curl(curl);

    if(res != CURLE_OK)
    {
        cout << "curl_request: Error on request: " << curl_easy_strerror(res) << endl;

        return "";
    }

    return response;
}

static void ExtractCallback(void *callback_data, size_t processed_entries, size_t total_entries)
{
    PROGRESSDATA* data = reinterpret_cast<PROGRESSDATA*>(callback_data);



    float progress = static_cast<float>(processed_entries) / static_cast<float>(total_entries);

    // Reduce the amount of updates/s
    if (progress - last_progress > 0.05)
    {
        *data->display_slider = ftxui::text(generate_slider(progress > 0.95 ? 1.0f : progress));
        data->screen->PostEvent(ftxui::Event::Custom);
    }
}

int display_extract(const string &filedata, const fs::path &destination_dir, ftxui::Element* display_slider, ftxui::ScreenInteractive* screen)
{
    last_progress = 0;

    PROGRESSDATA data;

    data.display_slider = display_slider;
    data.screen = screen;

    return extract_from_memory_to_folder(filedata, destination_dir, &data, ExtractCallback);
}
