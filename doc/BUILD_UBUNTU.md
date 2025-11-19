# Building S7Server on Ubuntu

This guide explains how to build and run the S7Server on Ubuntu Linux.

## Prerequisites

### Required Packages

Install the necessary build tools:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git wget
```

### System Requirements

- Ubuntu 20.04 LTS or later (or any Debian-based distribution)
- CMake 3.10 or higher
- GCC/G++ compiler
- Root/sudo access (for port 102)

## Quick Start

### Method 1: Using GitHub Repository (Recommended)

**Note:** The SourceForge download site does not have the latest version of Snap7. We recommend using the official GitHub repository.

1. **Clone Snap7 from GitHub**:
   ```bash
   cd /tmp
   git clone https://github.com/davenardella/snap7.git
   cd snap7
   ```

2. **Build Snap7**:
   ```bash
   cd build/linux
   make
   ```

3. **Copy Snap7 files to the project**:
   ```bash
   cd /path/to/S7-Server-Iso-on-tcp
   mkdir -p S7Server/snap7
   cp /tmp/snap7/release/wrappers/c-cpp/snap7.h S7Server/snap7/
   cp /tmp/snap7/build/bin/linux/libsnap7.so S7Server/snap7/
   ```

4. **Build S7Server**:
   ```bash
   ./build_ubuntu.sh
   ```

### Method 2: Using the Setup Script

If you have manually cloned the Snap7 repository to a different location:

1. **Clone Snap7** from https://github.com/davenardella/snap7.git to your preferred location

2. **Run the setup script** (you may need to modify the script to point to your Snap7 location):
   ```bash
   ./setup_snap7_linux.sh
   ```

3. **Build S7Server**:
   ```bash
   ./build_ubuntu.sh
   ```

### Method 3: Using Make

If you prefer using make:

1. **Ensure Snap7 files are in place** (see Method 1 above)

2. **Build using make**:
   ```bash
   make
   ```

   Or for debug build:
   ```bash
   make debug
   ```

   Other make targets:
   ```bash
   make clean      # Clean build artifacts
   make test       # Test the build
   make install    # Install system-wide (requires sudo)
   make help       # Show all available targets
   ```

### Method 4: Manual Build with CMake

If you prefer manual steps:

1. **Ensure Snap7 files are in place**:
   ```bash
   ls S7Server/snap7/
   # Should show: snap7.h  libsnap7.so
   ```

2. **Create build directory and compile**:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```

3. **The executable will be at**: `build/S7Server`

## Running the Server

### Important: Port 102 Requires Root Privileges

The S7 protocol uses TCP port 102, which requires root/sudo privileges on Linux.

**Run the server with sudo**:

```bash
sudo ./build/S7Server
```

### Server Output

When running successfully, you'll see:

```
========================================
S7 Server ISO-on-TCP (Snap7)
For Node-RED Testing
========================================

Initializing memory areas...
Memory areas registered successfully.
Starting server on port 102...
Note: Port 102 requires administrator privileges!

*** Server started successfully! ***

========================================
S7 Server Configuration:
========================================
Protocol: ISO-on-TCP
Port: 102
Data Blocks:
  - DB1: 256 bytes (General purpose)
  - DB2: 512 bytes (Extended data)
  - DB3: 128 bytes (Test data)
Inputs (I):  256 bytes
Outputs (Q): 256 bytes
Flags (M):   256 bytes
Timers (T):  512 bytes
Counters (C): 512 bytes
========================================

Server is running. Press Ctrl+C to stop.
```

### Using a Non-Privileged Port (Testing)

If you want to test without sudo, you can modify the code to use a non-privileged port (>1024):

Edit `S7Server/main.cpp` and uncomment these lines:

```cpp
// Configure server port (optional: use non-privileged port for testing)
int customPort = 10102;
Srv_SetParam(S7Server, p_u16_LocalPort, &customPort);
std::cout << "NOTE: Using custom port 10102 (no admin privileges required)" << std::endl;
```

Then rebuild and run without sudo:

```bash
./build_ubuntu.sh
./build/S7Server
```

**Note**: When using a custom port, you must configure your S7 client (Node-RED) to connect to that port instead of 102.

## Testing with Node-RED on Ubuntu

### Install Node-RED

```bash
# Install Node.js if not already installed
curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
sudo apt-get install -y nodejs

# Install Node-RED
sudo npm install -g --unsafe-perm node-red

# Install S7 contrib node
cd ~/.node-red
npm install node-red-contrib-s7
```

### Start Node-RED

```bash
node-red
```

Open your browser to `http://localhost:1880`

### Configure S7 Connection

In Node-RED:

1. Add an **S7 in** or **S7 out** node
2. Configure the connection:
   - **PLC Type**: S7-1200 or S7-1500
   - **IP Address**: `127.0.0.1` (or your server's IP)
   - **Port**: `102` (or your custom port)
   - **Rack**: `0`
   - **Slot**: `1`
   - **Connection Type**: ISO-on-TCP

3. Test with variables:
   - `DB1,INT0` - Should read value `42`
   - `DB1,BYTE1` - Should read value `100`
   - `DB2,BYTE0` - Should read value `1`

## Firewall Configuration

If you're running on a server and need to allow external connections:

### UFW (Ubuntu Firewall)

```bash
# Allow port 102
sudo ufw allow 102/tcp

# Or allow from specific IP
sudo ufw allow from 192.168.1.0/24 to any port 102
```

### iptables

```bash
# Allow port 102
sudo iptables -A INPUT -p tcp --dport 102 -j ACCEPT
```

## Troubleshooting

### Error: "Failed to start server"

**Problem**: Permission denied on port 102

**Solution**: Run with sudo
```bash
sudo ./build/S7Server
```

### Error: "cannot find -lsnap7"

**Problem**: Snap7 library not found during linking

**Solutions**:
1. Verify `S7Server/snap7/libsnap7.so` exists
2. Rebuild with: `./build_ubuntu.sh`
3. If using custom location, set: `export LD_LIBRARY_PATH=/path/to/snap7:$LD_LIBRARY_PATH`

### Error: "snap7.h: No such file or directory"

**Problem**: Snap7 header file not found

**Solution**: 
1. Clone Snap7 from the official GitHub repository: https://github.com/davenardella/snap7
2. Build Snap7 following the instructions in this guide
3. Copy `snap7.h` to `S7Server/snap7/snap7.h`

**Note:** The SourceForge download site does not have the latest version. Please use the GitHub repository.

### Error: "error while loading shared libraries: libsnap7.so"

**Problem**: Snap7 shared library not found at runtime

**Solutions**:

1. **Copy library to system path**:
   ```bash
   sudo cp S7Server/snap7/libsnap7.so /usr/local/lib/
   sudo ldconfig
   ```

2. **Or set LD_LIBRARY_PATH**:
   ```bash
   export LD_LIBRARY_PATH=$(pwd)/S7Server/snap7:$LD_LIBRARY_PATH
   sudo -E ./build/S7Server
   ```

3. **Or copy to build directory**:
   ```bash
   cp S7Server/snap7/libsnap7.so build/
   sudo ./build/S7Server
   ```

### Node-RED Cannot Connect

**Check**:

1. Server is running and shows "Server started successfully"
2. Firewall is not blocking port 102:
   ```bash
   sudo netstat -tlnp | grep 102
   ```
3. IP address and port are correct in Node-RED configuration
4. Server shows "Client connected" message when Node-RED tries to connect

## Build Options

### Debug Build

```bash
./build_ubuntu.sh Debug
```

### Release Build (default)

```bash
./build_ubuntu.sh Release
```

Or:
```bash
./build_ubuntu.sh
```

### Clean Build

```bash
rm -rf build
./build_ubuntu.sh
```

## Architecture Support

The build system supports:

- **x86_64** (64-bit Intel/AMD) - Default
- **i386** (32-bit Intel/AMD)
- **ARM v7** (32-bit ARM)
- **ARM64/AArch64** (64-bit ARM)

The build scripts will automatically detect your architecture and use the appropriate Snap7 build.

## Installation (Optional)

To install S7Server system-wide:

```bash
cd build
sudo cmake --install .
```

This will install:
- Executable to `/usr/local/bin/S7Server`
- Shared library to `/usr/local/lib/libsnap7.so`

Then you can run from anywhere:
```bash
sudo S7Server
```

## Docker Support

You can also run S7Server in a Docker container on Linux. This can simplify deployment and isolate the server in a container environment.

For Docker setup on Windows with Node-RED, see [DOCKER_SETUP.md](DOCKER_SETUP.md).

## Development

### Project Structure

```
S7-Server-Iso-on-tcp/
├── CMakeLists.txt          # CMake build configuration
├── build_ubuntu.sh         # Ubuntu build script
├── setup_snap7_linux.sh    # Snap7 setup script
├── S7Server/
│   ├── main.cpp            # Server implementation
│   └── snap7/              # Snap7 library files
│       ├── snap7.h         # Header file
│       └── libsnap7.so     # Shared library
└── build/                  # Build output directory
    └── S7Server            # Compiled executable
```

### Modifying the Code

After making changes to `main.cpp`:

```bash
./build_ubuntu.sh
sudo ./build/S7Server
```

## Performance Notes

- The Linux build uses pthreads for threading (vs Windows threads)
- Performance is typically better on Linux due to more efficient network stack
- For production use, consider running as a systemd service

## See Also

- [Main README](../README.md) - General project information
- [Windows Build Instructions](../README.md#setup-instructions) - Building on Windows
- [Docker Setup](DOCKER_SETUP.md) - Docker configuration
- [Snap7 Documentation](http://snap7.sourceforge.net/) - Snap7 library reference

## License

This project uses the Snap7 library which is licensed under LGPL v3.
See [Snap7 License](http://snap7.sourceforge.net/licensing.html) for details.
