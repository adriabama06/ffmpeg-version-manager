#include <filesystem>

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

int setup_env(std::string version);
int remove_env();
std::filesystem::path get_ffmpeg_vm_dir();

#endif // ENVIRONMENT_H