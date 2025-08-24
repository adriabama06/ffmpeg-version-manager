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

#ifdef _WIN32
#include <windows.h>

int update_windows_path(const fs::path& ffmpeg_vm_dir, bool add) {
    HKEY hKey;
    LONG lResult;
    DWORD dwType = REG_EXPAND_SZ;
    DWORD dwSize = 0;
    string currentPath;

    // Open environment key
    lResult = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey);
    if (lResult != ERROR_SUCCESS) {
        cerr << "Error: Could not open environment registry key" << endl;
        return 1;
    }

    // Get current PATH value
    lResult = RegQueryValueExA(hKey, "Path", NULL, &dwType, NULL, &dwSize);
    if (lResult == ERROR_SUCCESS) {
        vector<char> buffer(dwSize);
        lResult = RegQueryValueExA(hKey, "Path", NULL, &dwType, (LPBYTE)buffer.data(), &dwSize);
        if (lResult == ERROR_SUCCESS) {
            currentPath = string(buffer.data(), dwSize - 1); // Remove null terminator
        }
    }

    // Modify PATH
    string newPath;
    string dir_str = (ffmpeg_vm_dir / "bin").string();

    if (add) {
        if (currentPath.find(dir_str) != string::npos) {
            cout << "ffmpeg-vm already in PATH" << endl;
            RegCloseKey(hKey);
            return 0;
        }
        newPath = dir_str + ";" + currentPath;
    } else {
        size_t pos = currentPath.find(dir_str);
        if (pos == string::npos) {
            cout << "ffmpeg-vm not found in PATH" << endl;
            RegCloseKey(hKey);
            return 0;
        }
        newPath = currentPath;
        newPath.erase(pos, dir_str.length() + 1); // +1 to remove the semicolon
    }

    // Set new PATH value
    lResult = RegSetValueExA(hKey, "Path", 0, REG_EXPAND_SZ, 
                            (const BYTE*)newPath.c_str(), newPath.length() + 1);
    if (lResult != ERROR_SUCCESS) {
        cerr << "Error: Could not set PATH value" << endl;
        RegCloseKey(hKey);
        return 1;
    }

    RegCloseKey(hKey);

    // Notify system about environment change
    SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 
                       (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);

    return 0;
}
#endif

fs::path get_ffmpeg_vm()
{
    const char* user_ffmpeg_path = getenv("FFMPEGVM_PATH");
    const char* home = getenv(HOME);

    if (home == NULL) {
        cerr << "Error: " << HOME << " environment variable not set." << endl;
        return "";
    }

    fs::path ffmpeg_vm_dir = fs::path(user_ffmpeg_path != NULL ? user_ffmpeg_path : home) / "ffmpeg-vm";

    if (!fs::exists(ffmpeg_vm_dir)) {
        if (!fs::create_directories(ffmpeg_vm_dir)) {
            cerr << "Error: Could not create directory " << ffmpeg_vm_dir << endl;
            return "";
        }
    }

    return ffmpeg_vm_dir;
}

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

#ifdef _WIN32
    return update_windows_path(ffmpeg_vm_dir, true);
#else
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
    const string new_section = start_marker + "\nexport PATH=\"" + (ffmpeg_vm_dir / "bin").string() + ":" + (ffmpeg_vm_dir / "lib").string() + ":$PATH\"\n" + end_marker;

    if (content.find(start_marker) != string::npos) {
        cout << "ffmpeg-vm section already exists in .bashrc" << endl;
        return 4;
    }

    ofstream bashrc_out(bashrc_path, ios_base::app);
    if (!bashrc_out.is_open()) {
        cerr << "Error: Could not open " << bashrc_path << " for writing." << endl;
        return 5;
    }
    bashrc_out << "\n" << new_section << "\n";
    bashrc_out.close();

    return 0;
#endif
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
            cerr << "Error: Could not remove directory " << ffmpeg_vm_dir << endl;
        }
    }

#ifdef _WIN32
    return update_windows_path(ffmpeg_vm_dir, false);
#else
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
#endif
}
