#!/bin/bash

# Setup script for building Snap7 library on Ubuntu/Linux
# This script downloads and builds the Snap7 library from source

set -e  # Exit on error

echo "========================================"
echo "Snap7 Library Setup for Ubuntu"
echo "========================================"
echo ""

# Check if running on Linux
if [[ "$(uname -s)" != "Linux" ]]; then
    echo "ERROR: This script is for Linux only"
    echo "Detected OS: $(uname -s)"
    exit 1
fi

# Check for required tools
echo "Checking for required build tools..."

if ! command -v make &> /dev/null; then
    echo "ERROR: make is not installed"
    echo "Install it with: sudo apt-get install build-essential"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo "ERROR: g++ is not installed"
    echo "Install it with: sudo apt-get install build-essential"
    exit 1
fi

echo "✓ Build tools found"
echo ""

# Create snap7 directory
SNAP7_DIR="S7Server/snap7"
mkdir -p "$SNAP7_DIR"

# Check if snap7 source exists
TEMP_DIR="/tmp/snap7_build"
mkdir -p "$TEMP_DIR"

echo "========================================"
echo "MANUAL DOWNLOAD REQUIRED"
echo "========================================"
echo ""
echo "Due to network restrictions, please manually download Snap7:"
echo ""
echo "1. Visit: http://snap7.sourceforge.net/"
echo "2. Download the latest Snap7 release (e.g., snap7-full-1.4.2.tar.gz)"
echo "3. Save it to: $TEMP_DIR/snap7.tar.gz"
echo ""
echo "Then run this script again."
echo ""

SNAP7_ARCHIVE="$TEMP_DIR/snap7.tar.gz"

if [ ! -f "$SNAP7_ARCHIVE" ]; then
    echo "Snap7 archive not found at: $SNAP7_ARCHIVE"
    echo ""
    echo "Alternative: You can manually build Snap7:"
    echo "  1. Download and extract Snap7 from http://snap7.sourceforge.net/"
    echo "  2. Navigate to snap7-full-x.x.x/build/unix/"
    echo "  3. Run: make -f x86_64_linux.mk"
    echo "  4. Copy files to S7Server/snap7/:"
    echo "     - ../../release/Wrappers/c-cpp/snap7.h"
    echo "     - ../bin/x86_64-linux/libsnap7.so"
    echo ""
    exit 1
fi

echo "Found Snap7 archive, extracting..."
cd "$TEMP_DIR"
tar -xzf snap7.tar.gz

# Find the extracted directory
SNAP7_SRC=$(find . -maxdepth 1 -type d -name "snap7-full-*" | head -1)

if [ -z "$SNAP7_SRC" ]; then
    echo "ERROR: Could not find extracted Snap7 directory"
    exit 1
fi

echo "Building Snap7 library..."
cd "$SNAP7_SRC/build/unix"

# Determine architecture
ARCH=$(uname -m)
if [ "$ARCH" = "x86_64" ]; then
    MAKEFILE="x86_64_linux.mk"
elif [ "$ARCH" = "i686" ] || [ "$ARCH" = "i386" ]; then
    MAKEFILE="i386_linux.mk"
elif [ "$ARCH" = "armv7l" ] || [[ "$ARCH" == armv7* ]]; then
    MAKEFILE="arm_v7_linux.mk"
elif [ "$ARCH" = "aarch64" ]; then
    # ARM64/AArch64: Snap7 may use arm_v7 makefile or have a separate arm64 build
    # Check Snap7 documentation for your version to confirm architecture support
    MAKEFILE="arm_v7_linux.mk"
else
    echo "WARNING: Unknown architecture $ARCH, trying x86_64_linux.mk"
    MAKEFILE="x86_64_linux.mk"
fi

echo "Using makefile: $MAKEFILE"
make -f "$MAKEFILE"

if [ $? -ne 0 ]; then
    echo "ERROR: Snap7 build failed"
    exit 1
fi

echo "Copying Snap7 files to project..."

# Get absolute path to project
PROJECT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
DEST_DIR="$PROJECT_DIR/$SNAP7_DIR"

# Copy header file
cp "$TEMP_DIR/$SNAP7_SRC/release/Wrappers/c-cpp/snap7.h" "$DEST_DIR/"

# Copy library file - detect architecture-specific directory
if [ "$ARCH" = "x86_64" ]; then
    LIB_DIR="$TEMP_DIR/$SNAP7_SRC/build/bin/x86_64-linux"
elif [ "$ARCH" = "i686" ] || [ "$ARCH" = "i386" ]; then
    LIB_DIR="$TEMP_DIR/$SNAP7_SRC/build/bin/i386-linux"
elif [ "$ARCH" = "armv7l" ] || [[ "$ARCH" == armv7* ]] || [ "$ARCH" = "aarch64" ]; then
    LIB_DIR="$TEMP_DIR/$SNAP7_SRC/build/bin/arm_v7-linux"
else
    # Try to detect the actual build directory
    LIB_DIR=$(find "$TEMP_DIR/$SNAP7_SRC/build/bin" -maxdepth 1 -type d -name "*-linux" | head -1)
    if [ -z "$LIB_DIR" ]; then
        echo "ERROR: Could not find Snap7 build output directory"
        echo "Please manually copy libsnap7.so from snap7-full-x.x.x/build/bin/ to S7Server/snap7/"
        exit 1
    fi
fi

cp "$LIB_DIR/libsnap7.so" "$DEST_DIR/"

# Create static library copy (optional)
if [ -f "$LIB_DIR/libsnap7.a" ]; then
    cp "$LIB_DIR/libsnap7.a" "$DEST_DIR/"
fi

echo ""
echo "========================================"
echo "✓ Snap7 setup successful!"
echo "========================================"
echo ""
echo "Files installed:"
echo "  - $DEST_DIR/snap7.h"
echo "  - $DEST_DIR/libsnap7.so"
echo ""
echo "You can now run ./build_ubuntu.sh to build S7Server"
echo ""

# Cleanup
echo "Cleaning up temporary files..."
rm -rf "$TEMP_DIR"
