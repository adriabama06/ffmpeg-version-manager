#include <filesystem>

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

int setup_env();
int remove_env();
std::filesystem::path get_ffmpeg_vm();

#endif // ENVIRONMENT_H