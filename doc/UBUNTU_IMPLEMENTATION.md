# Ubuntu Build Support - Implementation Summary

This document provides a summary of the changes made to enable building and running S7Server on Ubuntu/Linux.

## Overview

The S7Server project was originally Windows-only, built with Visual Studio 2022. This update adds full support for building and running on Ubuntu and other Linux distributions using CMake, while maintaining backward compatibility with Windows builds.

## Changes Made

### 1. Build System Files

#### CMakeLists.txt
- Cross-platform CMake build configuration
- Supports both Windows (MSVC) and Linux (GCC) builds
- Automatically detects and links platform-specific libraries:
  - Windows: `snap7.lib`, `ws2_32.lib`, `winmm.lib`
  - Linux: `libsnap7.so` (or `.a`), `pthread`
- Handles library path configuration and RPATH for shared libraries

#### Makefile
- Provides a convenient make-based interface for Linux users
- Targets include: `build`, `debug`, `release`, `clean`, `install`, `test`, `help`
- Wraps CMake commands for easier use

#### build_ubuntu.sh
- Bash script for one-command building on Ubuntu
- Checks for required tools (cmake, g++)
- Validates Snap7 library presence
- Provides clear error messages and instructions
- Supports Debug and Release build types

#### setup_snap7_linux.sh
- Helps users build Snap7 from source on Linux
- Detects system architecture (x86_64, i386, ARM v7, ARM64)
- Automatically selects the correct makefile
- Copies built files to the project structure
- Includes comprehensive error handling

### 2. Documentation

#### BUILD_UBUNTU.md
- Complete guide for building on Ubuntu/Linux
- Multiple build methods:
  1. Pre-built Snap7 (manual download and build)
  2. Using setup script
  3. Using Make
  4. Manual CMake build
- Running instructions with sudo/root privileges
- Node-RED integration guide for Linux
- Firewall configuration (UFW, iptables)
- Comprehensive troubleshooting section
- Architecture support details

#### Updated README.md
- Added cross-platform support mention
- Updated Features section
- Added platform-specific Requirements sections
- Links to BUILD_UBUNTU.md for Linux users

#### Updated .gitignore
- Added `build/` directory to ignore CMake build artifacts
- Snap7 library files already ignored

### 3. Testing

The implementation was tested on Ubuntu 24.04 LTS:
- ✅ Successfully built Snap7 library from source
- ✅ Successfully built S7Server using all methods (script, make, cmake)
- ✅ Server starts and runs correctly on port 10102 (non-privileged)
- ✅ Server configuration displays correctly
- ✅ Graceful shutdown works (Ctrl+C)
- ✅ All build tools function as expected

## Build Methods for Users

Users now have multiple ways to build on Ubuntu:

1. **Quick build with script:**
   ```bash
   ./build_ubuntu.sh
   ```

2. **Using make:**
   ```bash
   make
   ```

3. **Manual with CMake:**
   ```bash
   mkdir build && cd build && cmake .. && make
   ```

## Platform Compatibility

### Windows (Unchanged)
- Visual Studio 2022
- .vcxproj build files
- Native Windows libraries

### Linux/Ubuntu (New)
- CMake 3.10+
- GCC/G++ compiler
- Pthreads for threading
- Snap7 shared or static library

### Architecture Support
- x86_64 (64-bit Intel/AMD)
- i386 (32-bit Intel/AMD)
- ARM v7 (32-bit ARM)
- ARM64/AArch64 (64-bit ARM)

## Key Features

1. **Cross-Platform**: Single codebase works on both Windows and Linux
2. **Flexible Build**: Multiple build methods to suit user preferences
3. **Well-Documented**: Comprehensive guides for all platforms
4. **Tested**: Verified working on Ubuntu 24.04 LTS
5. **No Code Changes**: The main.cpp remains unchanged and portable
6. **Backward Compatible**: Windows builds continue to work as before

## Known Limitations

1. **Snap7 Source**: Users must clone Snap7 from the GitHub repository (https://github.com/davenardella/snap7) as the SourceForge site does not have the latest version
2. **Port 102 Privilege**: Requires sudo/root on Linux (same as Windows requirement for Administrator)
3. **Library Building**: Users need to build Snap7 from source on Linux (or obtain pre-built binaries)

## Future Enhancements

Potential improvements for future versions:

1. Create Docker container for fully isolated Linux deployment
2. Add systemd service file for running as a daemon
3. Create pre-built packages (.deb, .rpm) with bundled Snap7
4. Add automated testing in CI/CD pipeline for Linux builds
5. Support for additional Unix-like systems (FreeBSD, macOS)

## Security Considerations

- No changes to the core C++ code, so security profile remains the same
- Build scripts validate inputs and provide clear error messages
- No secrets or credentials in build files
- All scripts use safe practices (exit on error, quoting variables)

## Conclusion

The S7Server now has full Ubuntu/Linux support with minimal changes to the codebase. The build system is flexible, well-documented, and tested. Users can choose their preferred build method and easily get the server running on Linux systems.
