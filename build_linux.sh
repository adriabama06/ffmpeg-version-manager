#!/bin/bash

cmake -B build

cmake --build build -j

mv build/ffmpeg-version-manager .
