cmake_minimum_required (VERSION 3.22)

project(ffmpeg-version-manager
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Static linking flags for Windows
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

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

# --- Fetch libcurl
FetchContent_Declare(curl
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG        curl-8_8_0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(curl)
# ---

# --- Fetch zlib (required for libarchive ZIP support)
FetchContent_Declare(zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.3.1
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(zlib)

# Make sure libarchive finds our zlib
set(ZLIB_ROOT ${zlib_BINARY_DIR})
set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR})
set(ZLIB_LIBRARY zlibstatic)
# ---

# --- Fetch libarchive
set(ENABLE_TEST OFF CACHE BOOL "Disable libarchive tests")
set(ENABLE_INSTALL OFF CACHE BOOL "Disable libarchive install")
set(ENABLE_ZLIB ON CACHE BOOL "Enable zlib support in libarchive")

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
  PRIVATE ${archive_SOURCE_DIR}/libarchive
)

target_link_libraries(ffmpeg-version-manager
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE libcurl
  PRIVATE archive_static
)

# Additional static linking for Windows
if(WIN32)
    target_link_options(ffmpeg-version-manager PRIVATE
        -static
        -static-libgcc 
        -static-libstdc++
    )
    
    # Link Windows system libraries statically
    target_link_libraries(ffmpeg-version-manager PRIVATE
        ws2_32
        wldap32
        winmm
        crypt32
        bcrypt
    )
endif()
