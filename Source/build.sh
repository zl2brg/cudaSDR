#!/bin/bash

# Exit on error
set -e

# Default Qt path - can be overridden by environment variable
QT_PATH=${CMAKE_PREFIX_PATH:-"~/Qt/6.9.3/gcc_64"}

echo "--- Configuring with Qt at: $QT_PATH ---"

# Create build directory if it doesn't exist
mkdir -p build

# Configure the project
cmake -B build -S . \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "--- Building ---"

# Build the project using all available cores
cmake --build build -j$(nproc)

echo "--- Done! Binary is in build/cudasdr ---"
