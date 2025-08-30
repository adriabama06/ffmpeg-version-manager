# FFmpeg Version Manager

Note: I made this code in a rush, this means that the code is hard to read.

A command-line tool to manage multiple versions of FFmpeg, enabling seamless switching between different versions for development and testing.

## Features

- Switch between pre-defined FFmpeg versions
- Pre-built binaries available for download (see [Releases](https://github.com/adriabama06/ffmpeg-version-manager/releases))
- Build from source for custom versions
- Cross-platform support (Linux, Windows)

## Installation

### Pre-built Binaries
Download the latest release from [GitHub Releases](https://github.com/adriabama06/ffmpeg-version-manager/releases) matching your OS.

### Building from Source
1. Clone the repository:
   ```bash
   git clone https://github.com/adriabama06/ffmpeg-version-manager.git
   cd ffmpeg-version-manager
   ```
2. Build for Linux:
   ```bash
   ./build_linux.sh
   ```
3. Build for Windows (using MSYS2 or similar):
   ```bash
   ./build_windows.sh
   ```

## Usage

After installation, use the `./ffmpeg-version-manager` command. Or on windows execute `ffmpeg-version-manager.exe`

## Contributing

Contributions are welcome! Open an issue or submit a pull request.

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.

## Thanks

This project uses the following libraries:

- [ftxui](https://github.com/arthursonzogni/ftxui.git)
- [nlohmann_json](https://github.com/nlohmann/json.git)
- [libcurl](https://github.com/curl/curl.git)
- [zlib](https://github.com/madler/zlib.git)
- [bzip2](https://github.com/commontk/bzip2.git)
- [xz](https://github.com/tukaani-project/xz.git)
- [libarchive](https://github.com/libarchive/libarchive.git)

And the builds from:
- https://github.com/BtbN/FFmpeg-Builds/releases
