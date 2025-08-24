# Toolchain file for cross-compiling to Windows 64-bit using MinGW
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compiler settings
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Static linking flags
set(CMAKE_C_FLAGS_INIT "-static")
set(CMAKE_CXX_FLAGS_INIT "-static")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -static-libgcc -static-libstdc++")

# Linker paths
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Host tools should come from the host system
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Libraries/headers should come from target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
