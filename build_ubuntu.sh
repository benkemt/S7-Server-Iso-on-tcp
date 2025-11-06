#!/bin/bash

# Build script for S7Server on Ubuntu
# This script compiles the S7Server with the Snap7 library

set -e  # Exit on error

echo "========================================"
echo "S7 Server - Ubuntu Build Script"
echo "========================================"
echo ""

# Check if running on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo "ERROR: This script is for Linux/Ubuntu only"
    exit 1
fi

# Check for required tools
echo "Checking for required build tools..."

if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed"
    echo "Install it with: sudo apt-get install cmake"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo "ERROR: g++ is not installed"
    echo "Install it with: sudo apt-get install build-essential"
    exit 1
fi

echo "✓ Build tools found"
echo ""

# Check for Snap7 library
SNAP7_DIR="S7Server/snap7"
SNAP7_HEADER="$SNAP7_DIR/snap7.h"
SNAP7_LIB_SO="$SNAP7_DIR/libsnap7.so"
SNAP7_LIB_A="$SNAP7_DIR/libsnap7.a"

echo "Checking for Snap7 library..."

if [ ! -f "$SNAP7_HEADER" ]; then
    echo "ERROR: Snap7 header file not found at $SNAP7_HEADER"
    echo ""
    echo "Please download Snap7 from http://snap7.sourceforge.net/"
    echo "Then copy the following files to $SNAP7_DIR/:"
    echo "  - snap7.h (from snap7-full-x.x.x/release/Wrappers/c-cpp/)"
    echo "  - libsnap7.so (from snap7-full-x.x.x/build/bin/x86_64-linux/)"
    echo "    OR"
    echo "  - libsnap7.a (from snap7-full-x.x.x/build/bin/x86_64-linux/)"
    echo ""
    echo "Alternatively, run ./setup_snap7_linux.sh to build Snap7 from source"
    exit 1
fi

if [ ! -f "$SNAP7_LIB_SO" ] && [ ! -f "$SNAP7_LIB_A" ]; then
    echo "ERROR: Snap7 library not found"
    echo "Expected either:"
    echo "  - $SNAP7_LIB_SO"
    echo "  - $SNAP7_LIB_A"
    echo ""
    echo "The library location depends on your system architecture."
    echo "Please download or build Snap7 library for your platform."
    echo "Run ./setup_snap7_linux.sh to build from source"
    exit 1
fi

echo "✓ Snap7 library found"
echo ""

# Set build type (default to Release)
BUILD_TYPE=${1:-Release}
echo "Build type: $BUILD_TYPE"
echo ""

# Create build directory
BUILD_DIR="build"
echo "Creating build directory..."
mkdir -p "$BUILD_DIR"

# Run CMake
echo "Running CMake configuration..."
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: CMake configuration failed"
    exit 1
fi

echo ""
echo "Building S7Server..."
cmake --build . --config $BUILD_TYPE

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed"
    exit 1
fi

cd ..

echo ""
echo "========================================"
echo "✓ Build successful!"
echo "========================================"
echo ""
echo "Executable location: $BUILD_DIR/S7Server"
echo ""
echo "To run the server:"
echo "  sudo $BUILD_DIR/S7Server"
echo ""
echo "Note: Port 102 requires root/sudo privileges"
echo ""
