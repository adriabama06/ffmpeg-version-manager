#ifndef _WIN32

#define HOME "HOME"

#include "environment.hh"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// If you using namespace std and windows.h it creates a confict in std::byte and windows byte
namespace fs = std::filesystem;

int os_setup_env(std::string version, std::filesystem::path ffmpeg_vm_dir, const char* home)
{
    fs::path bashrc_path = fs::path(home) / ".bashrc";
    std::ifstream bashrc_in(bashrc_path);
    if (!bashrc_in.is_open()) {
        std::cerr << "Error: Could not open " << bashrc_path << std::endl;
        return 3;
    }

    std::string content((std::istreambuf_iterator<char>(bashrc_in)), std::istreambuf_iterator<char>());
    bashrc_in.close();

    const std::string start_marker = "# --- ffmpeg-vm start ---";
    const std::string end_marker = "# --- ffmpeg-vm end ---";
    const std::string new_section = start_marker
        + "\nexport PATH=\"" + (ffmpeg_vm_dir / "bin").string() + ":$PATH\"\n"
        + "export FFMPEGVM_PATH=\"" + ffmpeg_vm_dir.string() + "\"\n"
        + end_marker;

    if (content.find(start_marker) != std::string::npos) {
        std::cout << "ffmpeg-vm section already exists in .bashrc" << std::endl;
        return 4;
    }

    std::ofstream bashrc_out(bashrc_path, std::ios_base::app);
    if (!bashrc_out.is_open()) {
        std::cerr << "Error: Could not open " << bashrc_path << " for writing." << std::endl;
        return 5;
    }
    bashrc_out << "\n" << new_section << "\n";
    bashrc_out.close();

    std::ofstream ffmpeg_version(ffmpeg_vm_dir / "VERSION");

    if (!ffmpeg_version.is_open()) {
        std::cerr << "Error: Could not open " << (ffmpeg_vm_dir / "VERSION") << " for writing." << std::endl;
        return 5;
    }

    ffmpeg_version << version << "\n";
    ffmpeg_version.close();

    return 0;
}

int os_remove_env(std::filesystem::path ffmpeg_vm_dir, const char* home)
{
    fs::path bashrc_path = fs::path(home) / ".bashrc";
    std::ifstream bashrc_in(bashrc_path);
    if (!bashrc_in.is_open()) {
        std::cerr << "Error: Could not open " << bashrc_path << std::endl;
        return 2;
    }

    std::string content((std::istreambuf_iterator<char>(bashrc_in)), std::istreambuf_iterator<char>());
    bashrc_in.close();

    const std::string start_marker = "# --- ffmpeg-vm start ---";
    const std::string end_marker = "# --- ffmpeg-vm end ---";

    size_t start_pos = content.find(start_marker);
    if (start_pos == std::string::npos) {
        std::cout << "No ffmpeg-vm section found in .bashrc" << std::endl;
        return 4;
    }
    size_t end_pos = content.find(end_marker, start_pos);
    if (end_pos == std::string::npos) {
        std::cout << "No end marker found after start marker in .bashrc" << std::endl;
        return 5;
    }
    end_pos += end_marker.length();

    // Handle newline characters to avoid extra empty lines
    if (end_pos < content.length() && content[end_pos] == '\n') {
        end_pos++;
    } else if (start_pos > 0 && content[start_pos - 1] == '\n') {
        start_pos--;
    }

    content.erase(start_pos - 1, end_pos - start_pos + 1); /* -1 and +1 to include the \n */

    std::ofstream bashrc_out(bashrc_path);
    if (!bashrc_out.is_open()) {
        std::cerr << "Error: Could not open " << bashrc_path << " for writing." << std::endl;
        return 6;
    }
    bashrc_out << content;
    bashrc_out.close();

    return 0;
}

#endif
