#include "cli.hh"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "environment.hh"
#include "request.hh"

using namespace std;

typedef struct OPTIONS_S {
    bool install;
    string install_arg;
    bool uninstall;
} OPTIONS;

OPTIONS default_options()
{
    return OPTIONS{
        install: false,
        install_arg: "",
        uninstall: false
    };
}

void print_help()
{
    cout << "ffmpeg-version-manager v0.1.4" << endl;
    cout << "Arguments:" << endl;
    cout << "   -h/--help              -> Print this screen." << endl;
    cout << "   -u/--uninstall         -> Uninstall the current ffmpeg-vm installed and the current env." << endl;
    cout << "   -i/--install <version> -> Install the version requested and creates the env." << endl;
    cout << "   -l/--list              -> Display the versions availables for your platform." << endl;
    cout << endl;
    cout << "Environment variables:" << endl;
#ifdef _WIN32
    cout << "   - FFMPEGVM_PATH        -> Change where ffmpeg is installed, by default is installed in \%USERPROFILE\%\\ffmpeg-vm" << endl;
#else
    cout << "   - FFMPEGVM_PATH        -> Change where ffmpeg is installed, by default is installed in $HOME/ffmpeg-vm" << endl;
#endif
    cout << "   - FFMPEGVM_URL         -> Change where ffmpeg versions are fetched, by default is " << FFMPEGVM_URL << endl;
    cout << endl;
    cout << "Thanks for using my program ;D - https://github.com/adriabama06/ffmpeg-version-manager" << endl;
}
void print_list()
{
    string FFMPEGVM_CURRENT_VERSION;

    {
        filesystem::path ffmpeg_vm_dir = get_ffmpeg_vm_dir();

        ifstream version_file(ffmpeg_vm_dir / "VERSION");
        if (version_file.is_open()) {
            string content((istreambuf_iterator<char>(version_file)), istreambuf_iterator<char>());
            version_file.close();

            for (size_t i = 0; i < content.length(); i++)
            {
                if(content[i] == '\n') break;
                FFMPEGVM_CURRENT_VERSION.push_back(content[i]);
            }
        }
    }

    cout << "Fetching versions..." << endl;

    vector<FFMPEG_VERSION> versions = get_ffmpeg_versions();

    cout << versions.size() << " loaded for this platform" << endl;

    cout << "Available versions:" << endl;

    for (const FFMPEG_VERSION& ver : versions)
    {
        if(!FFMPEGVM_CURRENT_VERSION.empty() && FFMPEGVM_CURRENT_VERSION == ver.version) cout << "- " << ver.version << " (Current)" << endl;
        else cout << "- " << ver.version << endl;
    }
}

int run_cli(int argc, char *argv[])
{
    OPTIONS options = default_options();

    for (int i = 1; i < argc; ++i) {
        string arg = string(argv[i]);

        if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        }
        else if (arg == "-i" || arg == "--install") {
            if (i + 1 >= argc) {
                std::cerr << "Error: missing <version> argument for " << arg << "\n";
                return 1;
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
            return 0;
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
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
        
        if(!valid_version)
        {
            cout << "The version " << options.install_arg << " is not available for your platform" << endl;

            return 1;
        }

        remove_env();

        setup_env(version.version);

        filesystem::path downloaddir = get_ffmpeg_vm_dir();

        cout << "Downloading ffmpeg " + version.version + "..." << endl;
        const string fdata = download_file(version.url, NULL, NULL);

        cout << "Extracting files..." << endl;
        extract(fdata, downloaddir, NULL, NULL);

#ifdef _WIN32
        cout << "Done! Please open a new terminal to load the custom env for ffmpeg-vm" << endl;
#else
        cout << "Done! Please reload your env using source ~/.bashrc or open a new terminal (Only the first time or if the custom env for ffmpeg-vm is not loaded)" << endl;
#endif
    }

    return 0;
}
