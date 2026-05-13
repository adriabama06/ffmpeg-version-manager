#include <filesystem>

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

int setup_env(std::string version);
int remove_env();
std::filesystem::path get_ffmpeg_vm_dir();

int os_setup_env(std::string version, std::filesystem::path ffmpeg_vm_dir, const char* home);
int os_remove_env(std::filesystem::path ffmpeg_vm_dir, const char* home);

#endif // ENVIRONMENT_H
