#include "cli.hh"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "environment.hh"
#include "request.hh"
#include "curl_tools.hh"
#include "compress.hh"
#include "ffmpeg-vm.hh"

using namespace std;

typedef struct OPTIONS_S {
    bool install = false;
    string install_arg = "";
    bool uninstall = false;
} OPTIONS;

void print_help()
{
    cout <<  PROGRAM_NAME << " v" << PROGRAM_VERSION << endl;
    cout << "Arguments:" << endl;
    cout << "   -h/--help              -> Print this screen." << endl;
    cout << "   -u/--uninstall         -> Uninstall the current ffmpeg-vm installed and the current env." << endl;
    cout << "   -i/--install <version> -> Install the version requested and creates the env. Also you can pass \"latest\" and it will automatically pick the last build for your os." << endl;
    cout << "   -l/--list              -> Display the versions availables for your platform." << endl;
    cout << endl;
    cout << "Environment variables:" << endl;
#ifdef _WIN32
    cout << "   - FFMPEGVM_PATH        -> Change where ffmpeg is installed, by default is installed in %%USERPROFILE%%\\ffmpeg-vm" << endl;
#else
    cout << "   - FFMPEGVM_PATH        -> Change where ffmpeg is installed, by default is installed in $HOME/ffmpeg-vm" << endl;
#endif
    cout << "   - FFMPEGVM_URL         -> Change where ffmpeg versions are fetched, by default is " << FFMPEGVM_URL << endl;
    cout << endl;
    cout << "Thanks for using my program ;D - https://github.com/adriabama06/ffmpeg-version-manager" << endl;
}

void print_list()
{
    string FFMPEGVM_CURRENT_VERSION = get_current_version();

    cout << "Fetching versions from '" << (getenv("FFMPEGVM_URL") != NULL ? getenv("FFMPEGVM_URL") : FFMPEGVM_URL) << "'..." << endl;

    vector<FFMPEG_VERSION> versions = get_ffmpeg_versions();

    cout << versions.size() << " loaded for this platform" << endl;

    cout << "Available versions:" << endl;

    for (const FFMPEG_VERSION& ver : versions)
    {
        if(!FFMPEGVM_CURRENT_VERSION.empty() && FFMPEGVM_CURRENT_VERSION == ver.version) cout << "- " << ver.version << " (Current)" << endl;
        else cout << "- " << ver.version << endl;
    }
}

CLI_EXIT_STATUS run_cli(int argc, char *argv[])
{
    OPTIONS options;

    for (int i = 1; i < argc; ++i) {
        string arg = string(argv[i]);

        if (arg == "-h" || arg == "--help") {
            print_help();
            return CLI_EXIT_STATUS::OK;
        }
        else if (arg == "-i" || arg == "--install") {
            if (i + 1 >= argc) {
                std::cerr << "Error: missing <version> argument for " << arg << "\n";
                return CLI_EXIT_STATUS::NO_ARGUMENT;
            }
            string version = argv[++i];
            options.install = true;
            options.install_arg = version;
        }
        else if (arg == "-u" || arg == "--uninstall") {
            options.uninstall = true;
        }
        else if (arg == "-l" || arg == "--list") {
            print_list();
            return CLI_EXIT_STATUS::OK;
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            return CLI_EXIT_STATUS::UNKNONW_OPTION;
        }
    }

    if(options.uninstall)
    {
        remove_env();

        cout << "Uninstall complete!" << endl;
    }

    if(options.install)
    {
        cout << "Fetching versions..." << endl;

        vector<FFMPEG_VERSION> versions = get_ffmpeg_versions();

        cout << versions.size() << " loaded for this platform" << endl;

        if(versions.size() == 0)
        {
            cout << "Wait, what? On what platform are you?? There are 0 versions for you???" << endl;
            cout << "Sorry, but I'm quitting..." << endl;
            return CLI_EXIT_STATUS::NO_VERSIONS_TO_INSTALL;
        }

        bool valid_version = false;
        FFMPEG_VERSION version;

        for (const FFMPEG_VERSION& ver : versions)
        {
            if(ver.version == options.install_arg)
            {
                version = ver;
                valid_version = true;
                break;
            }
        }
        
        if(options.install_arg == "latest")
        {
            valid_version = true;

            version = versions[0];
        }

        if(!valid_version)
        {
            cout << "The version " << options.install_arg << " is not available for your platform" << endl;

            return CLI_EXIT_STATUS::NO_VERSIONS_TO_INSTALL;
        }

        remove_env();

        setup_env(version.version);

        filesystem::path downloaddir = get_ffmpeg_vm_dir();

        cout << "Downloading ffmpeg " + version.version + "..." << endl;
        string fdata;
        int status = quick_curl_request(version.url, &fdata);

        if(status != 0)
        {
            cout << "Error on download the file" << endl;
            return CLI_EXIT_STATUS::ERROR_DOWNLOADING;
        }

        cout << "Extracting files..." << endl;
        status = extract_from_memory_to_folder(fdata, downloaddir);

        if(status != 0)
        {
            cout << "Error on extracting the file" << endl;
            return CLI_EXIT_STATUS::ERROR_EXTRACTING;
        }

#ifdef _WIN32
        cout << "Done! Please open a new terminal to load the custom env for ffmpeg-vm" << endl;
#else
        cout << "Done! Please reload your env using source ~/.bashrc or open a new terminal (Only the first time or if the custom env for ffmpeg-vm is not loaded)" << endl;
#endif
    }

    return CLI_EXIT_STATUS::OK;
}
