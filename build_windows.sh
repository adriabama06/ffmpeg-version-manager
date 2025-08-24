#!/bin/bash

# Configure with CMake - explicitly disable system package finding
cmake -B build-windows \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-windows.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_CURL_EXE=OFF \
  -DCURL_USE_OPENSSL=OFF \
  -DCURL_USE_SCHANNEL=ON \
  -DENABLE_TEST=OFF \
  -DENABLE_INSTALL=OFF \
  -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++" \
  -DCMAKE_CXX_FLAGS="-static -static-libgcc -static-libstdc++" \
  -DZLIB_ROOT="" \
  -DZLIB_INCLUDE_DIR="" \
  -DZLIB_LIBRARY=""

JOBS=$(nproc)

# Build with limited threads to avoid memory issues
cmake --build build-windows -j $((JOBS>8 ? 8 : JOBS))

cp build/ffmpeg-version-manager.exe .
