#ifdef _WIN32

#include "environment.hh"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// If you using namespace std and windows.h it creates a confict in std::byte and windows byte
namespace fs = std::filesystem;

#define HOME "USERPROFILE"

#include <windows.h>

int update_windows_path(const fs::path& ffmpeg_vm_dir, bool add) {
    HKEY hKey;
    LONG lResult;
    DWORD dwType = REG_EXPAND_SZ;
    DWORD dwSize = 0;
    std::string currentPath;

    // Open environment key
    lResult = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey);
    if (lResult != ERROR_SUCCESS) {
        std::cerr << "Error: Could not open environment registry key" << std::endl;
        return 1;
    }

    // Get current PATH value
    lResult = RegQueryValueExA(hKey, "Path", NULL, &dwType, NULL, &dwSize);
    if (lResult == ERROR_SUCCESS) {
        std::vector<char> buffer(dwSize);
        lResult = RegQueryValueExA(hKey, "Path", NULL, &dwType, (LPBYTE)buffer.data(), &dwSize);
        if (lResult == ERROR_SUCCESS) {
            currentPath = std::string(buffer.data(), dwSize - 1); // Remove null terminator
        }
    }

    // Modify PATH
    std::string newPath;
    std::string dir_str = (ffmpeg_vm_dir / "bin").string();

    if (add) {
        lResult = RegSetValueExA(hKey, "FFMPEGVM_PATH", 0, REG_EXPAND_SZ, 
                            (const BYTE*)ffmpeg_vm_dir.string().c_str(), ffmpeg_vm_dir.string().length() + 1);
        if(lResult == ERROR_SUCCESS)
        {
            SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 
                       (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
        }

        if (currentPath.find(dir_str) != std::string::npos) {
            std::cout << "ffmpeg-vm already in PATH" << std::endl;
            RegCloseKey(hKey);
            return 0;
        }
        newPath = dir_str + ";" + currentPath;
    } else {
        lResult = RegDeleteValueA(hKey, "FFMPEGVM_PATH");
        if(lResult == ERROR_SUCCESS)
        {
            SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 
                       (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
        }

        size_t pos = currentPath.find(dir_str);
        if (pos == std::string::npos) {
            std::cout << "ffmpeg-vm not found in PATH" << std::endl;
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
        std::cerr << "Error: Could not set PATH value" << std::endl;
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

#ifdef _WIN32

int os_setup_env(std::string version, std::filesystem::path ffmpeg_vm_dir, const char* home)
{
    int status = update_windows_path(ffmpeg_vm_dir, true);

    std::ofstream ffmpeg_version(ffmpeg_vm_dir / "VERSION");

    if (!ffmpeg_version.is_open()) {
        std::cerr << "Error: Could not open " << (ffmpeg_vm_dir / "VERSION") << " for writing." << std::endl;
        return 5;
    }

    ffmpeg_version << version << "\n";
    ffmpeg_version.close();

    return status;
}

int os_remove_env(std::filesystem::path ffmpeg_vm_dir, const char* home)
{
    return update_windows_path(ffmpeg_vm_dir, false);
}

#endif
