#!/bin/bash

# Setup script for building Snap7 library on Ubuntu/Linux
# This script clones and builds the Snap7 library from the official GitHub repository

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

if ! command -v git &> /dev/null; then
    echo "ERROR: git is not installed"
    echo "Install it with: sudo apt-get install git"
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
echo "Cloning Snap7 from GitHub"
echo "========================================"
echo ""
echo "NOTE: The SourceForge download site does not have the latest"
echo "version of Snap7. We will clone from the official GitHub repository:"
echo "https://github.com/davenardella/snap7"
echo ""

# Clone Snap7 from GitHub
cd "$TEMP_DIR"
if [ -d "snap7" ]; then
    echo "Removing existing snap7 directory..."
    rm -rf snap7
fi

echo "Cloning Snap7 repository..."
git clone https://github.com/davenardella/snap7.git

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Failed to clone Snap7 repository"
    echo ""
    echo "Alternative: You can manually clone and build Snap7:"
    echo "  1. Clone: git clone https://github.com/davenardella/snap7.git"
    echo "  2. Navigate to: snap7/build/unix/"
    echo "  3. Run: make -f x86_64_linux.mk"
    echo "  4. Copy files to S7Server/snap7/:"
echo "     - snap7/release/Wrappers/c-cpp/snap7.h"
    echo "     - snap7/build/bin/x86_64-linux/libsnap7.so"
    echo ""
    exit 1
fi

SNAP7_SRC="snap7"

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
        echo "Please manually copy libsnap7.so from snap7/build/bin/ to S7Server/snap7/"
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
echo "Snap7 cloned from: https://github.com/davenardella/snap7"
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
