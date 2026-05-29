#include <string>
#include <filesystem>

#ifndef COMPRESS_H
#define COMPRESS_H

int extract_from_memory_to_folder(const std::string &filedata, const std::filesystem::path &destination_dir, void* callback_data, void (*callback)(void*, size_t, size_t));
int extract_from_memory_to_folder(const std::string &filedata, const std::filesystem::path &destination_dir)
{
    return extract_from_memory_to_folder(filedata, destination_dir, NULL, NULL);
}

#endif // COMPRESS_H