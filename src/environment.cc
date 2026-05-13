#include "environment.hh"

#include <cstdlib>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>

// If you use namespace std & windows.h it creates a confict in std::byte and windows byte
namespace fs = std::filesystem;

#ifdef _WIN32
#define HOME "USERPROFILE"
#else
#define HOME "HOME"
#endif

fs::path get_ffmpeg_vm_dir()
{
    const char* user_ffmpeg_path = getenv("FFMPEGVM_PATH");
    const char* home = getenv(HOME);

    if (home == NULL) {
        std::cerr << "Error: " << HOME << " environment variable not set." << std::endl;
        return "";
    }

    fs::path ffmpeg_vm_dir = user_ffmpeg_path != NULL ? fs::path(user_ffmpeg_path) : (fs::path(home) / "ffmpeg-vm");

    if (!fs::exists(ffmpeg_vm_dir)) {
        if (!fs::create_directories(ffmpeg_vm_dir)) {
            std::cerr << "Error: Could not create directory " << ffmpeg_vm_dir << std::endl;
            return "";
        }
    }

    return ffmpeg_vm_dir;
}

int setup_env(std::string version)
{
    const char* home = getenv(HOME);

    if (home == NULL) {
        std::cerr << "Error: " << HOME << " environment variable not set." << std::endl;
        return 1;
    }

    fs::path ffmpeg_vm_dir = get_ffmpeg_vm_dir();

    if (!fs::exists(ffmpeg_vm_dir)) {
        if (!fs::create_directories(ffmpeg_vm_dir)) {
            std::cerr << "Error: Could not create directory " << ffmpeg_vm_dir << std::endl;
            return 2;
        }
    }

    return os_setup_env(version, ffmpeg_vm_dir, home);
}

int remove_env()
{
    const char* home = getenv(HOME);

    if (home == NULL) {
        std::cerr << "Error: " << HOME << " environment variable not set." << std::endl;
        return 1;
    }

    fs::path ffmpeg_vm_dir = get_ffmpeg_vm_dir();

    if (fs::exists(ffmpeg_vm_dir)) {
        if (!fs::remove_all(ffmpeg_vm_dir)) {
            std::cerr << "Error: Could not remove directory " << ffmpeg_vm_dir << std::endl;
        }
    }

    return os_remove_env(ffmpeg_vm_dir, home);
}
