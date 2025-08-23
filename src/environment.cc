#include "environment.hh"

#include <cstdlib>

#include <filesystem>
#include <iostream>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

#ifdef _WIN32
#define HOME "USERPROFILE"
#else
#define HOME "HOME"
#endif

int setup_env()
{
    const char* user_ffmpeg_path = getenv("FFMPEGVM_PATH");

    const char* home = getenv(HOME);

    if (home == NULL) {
        cerr << "Error: " << HOME << " environment variable not set." << endl;
        return 1;
    }

    fs::path ffmpeg_vm_dir = fs::path(user_ffmpeg_path != NULL ? user_ffmpeg_path : home) / "ffmpeg-vm";

    if (!fs::exists(ffmpeg_vm_dir)) {
        if (!fs::create_directories(ffmpeg_vm_dir)) {
            cerr << "Error: Could not create directory " << ffmpeg_vm_dir << endl;
            return 2;
        }
    }

    fs::path bashrc_path = fs::path(home) / ".bashrc";
    ifstream bashrc_in(bashrc_path);
    if (!bashrc_in.is_open()) {
        cerr << "Error: Could not open " << bashrc_path << endl;
        return 3;
    }

    string content((istreambuf_iterator<char>(bashrc_in)), istreambuf_iterator<char>());

    bashrc_in.close();

    const string start_marker = "# --- ffmpeg-vm start ---";
    const string end_marker = "# --- ffmpeg-vm end ---";
    const string new_section = start_marker + "\nexport PATH=\"" + ffmpeg_vm_dir.string() + ":$PATH\"\n" + end_marker;

    if (content.find(start_marker) != string::npos) {
        cout << "ffmpeg-vm section already exists in .bashrc" << endl;
        return 4;
    }

    ofstream bashrc_out(bashrc_path, ios_base::app);
    if (!bashrc_out.is_open()) {
        std::cerr << "Error: Could not open " << bashrc_path << " for writing." << std::endl;
        return 5;
    }
    bashrc_out << "\n" << new_section << "\n";
    bashrc_out.close();

    return 0;
}

int remove_env()
{
    const char* user_ffmpeg_path = getenv("FFMPEGVM_PATH");

    const char* home = getenv(HOME);

    if (home == NULL) {
        cerr << "Error: " << HOME << " environment variable not set." << endl;
        return 1;
    }

    fs::path ffmpeg_vm_dir = fs::path(user_ffmpeg_path != NULL ? user_ffmpeg_path : home) / "ffmpeg-vm";

    if (fs::exists(ffmpeg_vm_dir)) {
        if (!fs::remove_all(ffmpeg_vm_dir)) {
            std::cerr << "Error: Could not remove directory " << ffmpeg_vm_dir << std::endl;
        }
    }

    // Update .bashrc
    fs::path bashrc_path = fs::path(home) / ".bashrc";
    ifstream bashrc_in(bashrc_path);
    if (!bashrc_in.is_open()) {
        cerr << "Error: Could not open " << bashrc_path << endl;
        return 2;
    }

    string content((istreambuf_iterator<char>(bashrc_in)), istreambuf_iterator<char>());
    bashrc_in.close();

    const string start_marker = "# --- ffmpeg-vm start ---";
    const string end_marker = "# --- ffmpeg-vm end ---";

    size_t start_pos = content.find(start_marker);
    if (start_pos == string::npos) {
        cout << "No ffmpeg-vm section found in .bashrc" << endl;
        return 4;
    }
    size_t end_pos = content.find(end_marker, start_pos);
    if (end_pos == string::npos) {
        cout << "No end marker found after start marker in .bashrc" << endl;
        return 5;
    }
    end_pos += end_marker.length();

    // Handle newline characters to avoid extra empty lines
    if (end_pos < content.length() && content[end_pos] == '\n') {
        end_pos++;
    } else if (start_pos > 0 && content[start_pos - 1] == '\n') {
        start_pos--;
    }

    content.erase(start_pos, end_pos - start_pos);

    ofstream bashrc_out(bashrc_path);
    if (!bashrc_out.is_open()) {
        cerr << "Error: Could not open " << bashrc_path << " for writing." << endl;
        return 6;
    }
    bashrc_out << content;
    bashrc_out.close();

    return 0;
}
