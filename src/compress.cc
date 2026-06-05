#include "compress.hh"

#include <iostream>
#include <fstream>

#include <archive.h>
#include <archive_entry.h>

using namespace std;
namespace fs = std::filesystem;

int extract_from_memory_to_folder(const std::string &filedata, const std::filesystem::path &destination_dir, void* callback_data, void (*callback)(void*, size_t, size_t))
{
    struct archive *archiv;
    struct archive_entry *entry;
    int result;

    archiv = archive_read_new();
    archive_read_support_filter_all(archiv); // Support for gzip, bzip2, xz, etc.
    archive_read_support_format_all(archiv); // Support for tar, zip, 7zip, etc.

    // Load from memory
    result = archive_read_open_memory(archiv, filedata.data(), filedata.size());

    if (result != ARCHIVE_OK)
    {
        cerr << "Error opening archive from memory: " << archive_error_string(archiv) << endl;
        archive_read_free(archiv);
        return 1;
    }

    // Number of chars to remove at the start of the path
    size_t ffmpeg_entry_path_lenght = 0;

    size_t total_entries = 0;
    size_t processed_entries = 0;

    // Read all entry to know how many files are
    while (archive_read_next_header(archiv, &entry) == ARCHIVE_OK) {
        total_entries++;
        archive_read_data_skip(archiv); // Saltar los datos para solo contar
    }
    
    // Restart the read
    archive_read_free(archiv);
    archiv = archive_read_new();
    archive_read_support_filter_all(archiv);
    archive_read_support_format_all(archiv);
    result = archive_read_open_memory(archiv, filedata.data(), filedata.size());

    if (result != ARCHIVE_OK)
    {
        cerr << "Error reopening archive from memory: " << archive_error_string(archiv) << endl;
        archive_read_free(archiv);
        return 1;
    }

    // Read every entry (file/dir) inside the compressed file
    while (archive_read_next_header(archiv, &entry) == ARCHIVE_OK)
    {
        if (total_entries > 0 && callback != NULL)
        {
            processed_entries++;

            callback(callback_data, processed_entries, total_entries);
        }

        fs::path entry_path = fs::path(archive_entry_pathname(entry));

        if (ffmpeg_entry_path_lenght != 0)
        {
            string temp = entry_path.string();

            temp.erase(0, ffmpeg_entry_path_lenght);

            entry_path = fs::path(temp);
        }

        const fs::path full_dest_path = destination_dir / entry_path;

        /*
            The first entry is the root folder
            because we want all in ffmpeg-vm/... and not in ffmpeg-vm/something/..., we check this path to remove
        */
        if (ffmpeg_entry_path_lenght == 0 && entry_path.string().rfind("ffmpeg") == 0) // Check if starts with ffmpeg
        {
            ffmpeg_entry_path_lenght = entry_path.string().length();
            continue;
        }

        // Make sure that the folder exist
        if (full_dest_path.has_parent_path())
        {
            fs::create_directories(full_dest_path.parent_path());
        }

        // If is only a directory, create the dir
        if (archive_entry_filetype(entry) == AE_IFDIR)
        {
            fs::create_directories(full_dest_path);

            continue;
        }

#ifndef _WIN32
        // If is only a symlink, create the symlink
        if (archive_entry_filetype(entry) == AE_IFLNK)
        {
            const char* link_target_cstr = archive_entry_symlink(entry);

            if (link_target_cstr) {
                // fs::create_symlink can fail if the link exist
                std::error_code ec;
                fs::remove(full_dest_path, ec); // Remove to prevent errors
                fs::create_symlink(fs::path(link_target_cstr), full_dest_path);
            } else {
                cerr << "Warning: could not read symlink target for " << full_dest_path << endl;
            }

            continue;
        }
#endif

        // if it's a file, write it to disk
        ofstream outfile(full_dest_path, ios::binary);

        if (!outfile)
        {
            cerr << "Error: Could not open file for writing: " << full_dest_path << endl;
            archive_read_close(archiv);
            archive_read_free(archiv);
            return 2;
        }

        const void *buff; // Points to the data, no need to free, is only a pointer in memory
        size_t size;
        la_int64_t offset;

        // Read the blocks and write it
        while ((result = archive_read_data_block(archiv, &buff, &size, &offset)) == ARCHIVE_OK)
        {
            outfile.write(static_cast<const char *>(buff), size);
        }

        if (result != ARCHIVE_EOF)
        {
            cerr << "Error reading data block: " << archive_error_string(archiv) << endl;
            archive_read_close(archiv);
            archive_read_free(archiv);
            return 3;
        }

#ifndef _WIN32
        // Restore permissions
        if (archive_entry_filetype(entry) != AE_IFLNK) {
            try {
                fs::permissions(full_dest_path, static_cast<fs::perms>(archive_entry_perm(entry)), fs::perm_options::replace);
            } catch (const exception& e) {
                cerr << "Warning: could not set permissions for " << full_dest_path << ". " << e.what() << endl;
            }
        }
#endif
    }

    // Check if error on readling last header
    result = archive_read_close(archiv);

    if (result != ARCHIVE_OK)
    {
        cerr << "Error closing archive: " << archive_error_string(archiv) << endl;
        archive_read_free(archiv);
        return 4;
    }

    archive_read_free(archiv);

    return 0;
}
