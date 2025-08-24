#!/bin/bash

# Configure with CMake
cmake -B build-windows \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-windows.cmake

# Build with all cores
cmake --build build-windows -j
