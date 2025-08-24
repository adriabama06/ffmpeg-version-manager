cmake_minimum_required (VERSION 3.22)

project(ffmpeg-version-manager
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v5.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
# ------------------------------------------------------------------------------

# --- Fetch JSON
FetchContent_Declare(nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.12.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(nlohmann_json)
# ---

# --- Find libcurl
find_package(CURL REQUIRED)
# ---

# --- Fetch libarchive
set(ENABLE_TEST OFF)
FetchContent_Declare(archive
  GIT_REPOSITORY https://github.com/libarchive/libarchive.git
  GIT_TAG        v3.8.1
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(archive)
# ---

add_executable(ffmpeg-version-manager
  src/main.cc
  src/request.cc
  src/environment.cc
)

target_include_directories(ffmpeg-version-manager
  PRIVATE src
)

target_link_libraries(ffmpeg-version-manager
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component  # Not needed for this example.
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE CURL::libcurl
  PRIVATE archive
)
