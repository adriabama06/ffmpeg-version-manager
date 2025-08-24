#!/bin/bash

# Configure with CMake for static linking
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++" \
  -DCMAKE_CXX_FLAGS="-static -static-libgcc -static-libstdc++"

JOBS=$(nproc)

# Build with limited threads to avoid memory issues
cmake --build build -j $((JOBS>8 ? 8 : JOBS))

cp build/ffmpeg-version-manager .

