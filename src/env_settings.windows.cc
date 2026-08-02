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

static std::string get_registry_value(HKEY root_key, const std::string& sub_key, const std::string& value_name)
{
    HKEY hKey;
    std::string result;
    LONG lResult = RegOpenKeyExA(root_key, sub_key.c_str(), 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) return result;

    DWORD dwType = REG_EXPAND_SZ;
    DWORD dwSize = 0;
    lResult = RegQueryValueExA(hKey, value_name.c_str(), NULL, &dwType, NULL, &dwSize);
    if (lResult == ERROR_SUCCESS && dwSize > 0) {
        std::vector<char> buffer(dwSize);
        lResult = RegQueryValueExA(hKey, value_name.c_str(), NULL, &dwType, (LPBYTE)buffer.data(), &dwSize);
        if (lResult == ERROR_SUCCESS) {
            result = std::string(buffer.data(), dwSize - 1);
        }
    }

    RegCloseKey(hKey);
    return result;
}

static LONG set_registry_key(HKEY root_key, const std::string& sub_key, const std::string& value_name, const std::string& value)
{
    HKEY hKey;
    LONG lResult = RegOpenKeyExA(root_key, sub_key.c_str(), 0, KEY_WRITE, &hKey);
    if (lResult != ERROR_SUCCESS) return lResult;

    lResult = RegSetValueExA(hKey, value_name.c_str(), 0, REG_EXPAND_SZ,
                             (const BYTE*)value.c_str(), static_cast<DWORD>(value.length() + 1));

    RegCloseKey(hKey);
    return lResult;
}

static LONG delete_registry_key(HKEY root_key, const std::string& sub_key, const std::string& value_name)
{
    HKEY hKey;
    LONG lResult = RegOpenKeyExA(root_key, sub_key.c_str(), 0, KEY_WRITE, &hKey);
    if (lResult != ERROR_SUCCESS) return lResult;

    lResult = RegDeleteValueA(hKey, value_name.c_str());

    RegCloseKey(hKey);
    return lResult;
}

static void notify_environment_change()
{
    SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                       (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
}

int set_windows_path(const fs::path& ffmpeg_vm_dir)
{
    std::string dir_str = (ffmpeg_vm_dir / "bin").string();
    std::string currentPath = get_registry_value(HKEY_CURRENT_USER, "Environment", "Path");

    LONG lResult = set_registry_key(HKEY_CURRENT_USER, "Environment", "FFMPEGVM_PATH", ffmpeg_vm_dir.string());
    if (lResult == ERROR_SUCCESS) {
        notify_environment_change();
    }

    if (currentPath.find(dir_str) != std::string::npos) {
        std::cout << "ffmpeg-vm already in PATH" << std::endl;
        return 0;
    }

    std::string newPath = dir_str + ";" + currentPath;
    lResult = set_registry_key(HKEY_CURRENT_USER, "Environment", "Path", newPath);
    if (lResult != ERROR_SUCCESS) {
        std::cerr << "Error: Could not set PATH value" << std::endl;
        return 1;
    }

    notify_environment_change();
    return 0;
}

int delete_windows_path(const fs::path& ffmpeg_vm_dir)
{
    std::string dir_str = (ffmpeg_vm_dir / "bin").string();
    std::string currentPath = get_registry_value(HKEY_CURRENT_USER, "Environment", "Path");

    LONG lResult = delete_registry_key(HKEY_CURRENT_USER, "Environment", "FFMPEGVM_PATH");
    if (lResult == ERROR_SUCCESS) {
        notify_environment_change();
    }

    size_t pos = currentPath.find(dir_str);
    if (pos == std::string::npos) {
        std::cout << "ffmpeg-vm not found in PATH" << std::endl;
        return 0;
    }

    std::string newPath = currentPath;
    newPath.erase(pos, dir_str.length() + 1);

    lResult = set_registry_key(HKEY_CURRENT_USER, "Environment", "Path", newPath);
    if (lResult != ERROR_SUCCESS) {
        std::cerr << "Error: Could not set PATH value" << std::endl;
        return 1;
    }

    notify_environment_change();
    return 0;
}

int os_setup_env(std::string version, std::filesystem::path ffmpeg_vm_dir, [[maybe_unused]] const char* home)
{
    int status = set_windows_path(ffmpeg_vm_dir);

    std::ofstream ffmpeg_version(ffmpeg_vm_dir / "VERSION");

    if (!ffmpeg_version.is_open()) {
        std::cerr << "Error: Could not open " << (ffmpeg_vm_dir / "VERSION") << " for writing." << std::endl;
        return 5;
    }

    ffmpeg_version << version << "\n";
    ffmpeg_version.close();

    return status;
}

int os_remove_env(std::filesystem::path ffmpeg_vm_dir, [[maybe_unused]] const char* home)
{
    return delete_windows_path(ffmpeg_vm_dir);
}

#endif
