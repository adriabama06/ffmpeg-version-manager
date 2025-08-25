#include "request.hh"

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include <curl/curl.h>

#include <nlohmann/json.hpp>

#include <archive.h>
#include <archive_entry.h>

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

#define FFMPEG_LIST_URL "https://raw.githubusercontent.com/adriabama06/ffmpeg-version-manager/refs/heads/main/ffmpeg-list.json"

// Callback function to write curl response to a string
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

vector<FFMPEG_VERSION> get_ffmpeg_versions()
{
    CURL *curl;
    CURLcode res;
    string response;

    curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize curl" << endl;
        return {};
    }

    curl_easy_setopt(curl, CURLOPT_URL, FFMPEG_LIST_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_easy_cleanup(curl);
        return {};
    }

    curl_easy_cleanup(curl);

    // Error on request
    if (response.empty())
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

        raw_list.push_back(FFMPEG_VERSION{version : key, url : url});
    }

    // versions.items() & raw_list is sorted, so to reverse start from the end
    vector<FFMPEG_VERSION> list(raw_list.size());

    for (size_t i = 0; i < raw_list.size(); i++)
    {
        list[raw_list.size() - i - 1] = raw_list[i];
    }

    return list;
}

string download_file(string url)
{
    // TODO: Add download progress: https://github.com/dryark/minibrew_deploy/blob/main/curlprog.m#L31
    CURL *curl;
    CURLcode res;
    string response;

    curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize curl" << endl;
        return "";
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        curl_easy_cleanup(curl);
        return "";
    }

    curl_easy_cleanup(curl);
    return response;
}

int extract(const string &filedata, const fs::path &destination_dir)
{
    struct archive *archiv;
    struct archive_entry *entry;
    int result;

    archiv = archive_read_new();
    archive_read_support_filter_all(archiv); // Support for gzip, bzip2, xz, etc.
    archive_read_support_format_all(archiv); // Support for tar, zip, 7zip, etc.

    // Load from memory
    result = archive_read_open_memory(archiv, filedata.data(), filedata.size());

    if (result != ARCHIVE_OK)
    {
        cerr << "Error opening archive from memory: " << archive_error_string(archiv) << endl;
        archive_read_free(archiv);
        return 1;
    }

    // Number of chars to remove at the start of the path
    size_t ffmpeg_entry_path_lenght = 0;

    // Read every entry (file/dir) inside the compressed file
    while (archive_read_next_header(archiv, &entry) == ARCHIVE_OK)
    {
        fs::path entry_path = fs::path(archive_entry_pathname(entry));

        if (ffmpeg_entry_path_lenght != 0)
        {
            string temp = entry_path.string();

            temp.erase(0, ffmpeg_entry_path_lenght);

            entry_path = fs::path(temp);
        }

        const fs::path full_dest_path = destination_dir / entry_path;

        /*
            The first entry is the root folder
            because we want all in ffmpeg-vm/... and not in ffmpeg-vm/something/..., we check this path to remove
        */
        if (ffmpeg_entry_path_lenght == 0 && entry_path.string().rfind("ffmpeg") == 0) // Check if starts with ffmpeg
        {
            ffmpeg_entry_path_lenght = entry_path.string().length();
            continue;
        }

        // Make sure that the folder exist
        if (full_dest_path.has_parent_path())
        {
            fs::create_directories(full_dest_path.parent_path());
        }

        // If is only a directory, create the dir
        if (archive_entry_filetype(entry) == AE_IFDIR)
        {
            fs::create_directories(full_dest_path);

            continue;
        }

#ifndef _WIN32
        // If is only a symlink, create the symlink
        if (archive_entry_filetype(entry) == AE_IFLNK)
        {
            const char* link_target_cstr = archive_entry_symlink(entry);

            if (link_target_cstr) {
                // fs::create_symlink can fail if the link exist
                std::error_code ec;
                fs::remove(full_dest_path, ec); // Remove to prevent errors
                fs::create_symlink(fs::path(link_target_cstr), full_dest_path);
            } else {
                cerr << "Warning: could not read symlink target for " << full_dest_path << endl;
            }

            continue;
        }
#endif

        // if it's a file, write it to disk
        ofstream outfile(full_dest_path, ios::binary);

        if (!outfile)
        {
            cerr << "Error: Could not open file for writing: " << full_dest_path << endl;
            archive_read_close(archiv);
            archive_read_free(archiv);
            return 2;
        }

        const void *buff;
        size_t size;
        la_int64_t offset;

        // Read the blocks and write it
        while ((result = archive_read_data_block(archiv, &buff, &size, &offset)) == ARCHIVE_OK)
        {
            outfile.write(static_cast<const char *>(buff), size);
        }

        if (result != ARCHIVE_EOF)
        {
            cerr << "Error reading data block: " << archive_error_string(archiv) << endl;
            archive_read_close(archiv);
            archive_read_free(archiv);
            return 3;
        }

#ifndef _WIN32
        // Restore permissions
        if (archive_entry_filetype(entry) != AE_IFLNK) {
            try {
                fs::permissions(full_dest_path, static_cast<fs::perms>(archive_entry_perm(entry)), fs::perm_options::replace);
            } catch (const exception& e) {
                cerr << "Warning: could not set permissions for " << full_dest_path << ". " << e.what() << endl;
            }
        }
#endif
    }

    // Check if error on readling last header
    result = archive_read_close(archiv);

    if (result != ARCHIVE_OK)
    {
        cerr << "Error closing archive: " << archive_error_string(archiv) << endl;
        archive_read_free(archiv);
        return 4;
    }

    archive_read_free(archiv);

    return 0;
}
