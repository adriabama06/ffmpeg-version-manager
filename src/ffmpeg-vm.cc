#include "ffmpeg-vm.hh"

#include "environment.hh"

#include <filesystem>
#include <fstream>

std::string get_current_version()
{
    std::string result;

    std::filesystem::path ffmpeg_vm_dir = get_ffmpeg_vm_dir();

    std::ifstream version_file(ffmpeg_vm_dir / "VERSION");

    if (version_file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(version_file)), std::istreambuf_iterator<char>());
        version_file.close();

        for (size_t i = 0; i < content.length(); i++)
        {
            if(content[i] == '\n') break; // Remove \n or \n\r
            result.push_back(content[i]);
        }
    }

    return result;
}
