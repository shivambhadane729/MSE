#!/bin/bash

# AMSE Run Helper for WSL
# This script ensures the project is built and runs with optimal settings.

set -e

# 1. Navigate to the cpp directory
cd "$(dirname "$0")"

# 2. Build Check
mkdir -p build && cd build
if [ ! -f "Makefile" ]; then
    echo "[INFO] Generating build files..."
    cmake ..
fi

echo "[INFO] Building amse_cpp..."
make -j$(nproc)

# 3. Environment Optimization for WSL
# If DISPLAY is set but QT_QPA_PLATFORM is not, default to xcb
if [ -n "$DISPLAY" ] && [ -z "$QT_QPA_PLATFORM" ]; then
    export QT_QPA_PLATFORM=xcb
fi

# 4. Run the application
echo "[INFO] Launching AMSE..."
./amse_cpp "$@"
