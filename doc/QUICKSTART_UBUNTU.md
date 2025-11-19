# Quick Start - Ubuntu/Linux

Get S7Server running on Ubuntu in just a few steps!

## Prerequisites

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git wget
```

## Option 1: Build Snap7 Manually (Recommended)

```bash
# 1. Download Snap7
cd /tmp
wget http://sourceforge.net/projects/snap7/files/1.4.2/snap7-full-1.4.2.tar.gz

# 2. Extract and build
tar -xzf snap7-full-1.4.2.tar.gz
cd snap7-full-1.4.2/build/unix
make -f x86_64_linux.mk

# 3. Copy to project
cd /path/to/S7-Server-Iso-on-tcp
mkdir -p S7Server/snap7
cp /tmp/snap7-full-1.4.2/release/Wrappers/c-cpp/snap7.h S7Server/snap7/
cp /tmp/snap7-full-1.4.2/build/bin/x86_64-linux/libsnap7.so S7Server/snap7/

# 4. Build S7Server
make

# 5. Run (requires sudo for port 102)
sudo ./build/S7Server
```

## Option 2: Using Build Script

```bash
# After setting up Snap7 (steps 1-3 above)
./build_ubuntu.sh
sudo ./build/S7Server
```

## Testing Without sudo

Edit `S7Server/main.cpp` and uncomment lines 161-163:
```cpp
int customPort = 10102;
Srv_SetParam(S7Server, p_u16_LocalPort, &customPort);
std::cout << "NOTE: Using custom port 10102 (no admin privileges required)" << std::endl;
```

Then rebuild and run:
```bash
make
./build/S7Server
```

**Note:** When using port 10102, configure your S7 client to connect to this port instead of 102.

## Connect from Node-RED

```bash
# Install Node-RED and S7 node
npm install -g node-red
cd ~/.node-red
npm install node-red-contrib-s7

# Start Node-RED
node-red
```

Configure S7 node:
- IP: 127.0.0.1
- Port: 102 (or 10102 if using custom port)
- Rack: 0
- Slot: 1

Test with variable: `DB1,INT0` (should read 42)

## Troubleshooting

### Permission Denied on Port 102
Run with sudo: `sudo ./build/S7Server`

### Library Not Found
```bash
sudo cp S7Server/snap7/libsnap7.so /usr/local/lib/
sudo ldconfig
```

### Build Errors
Check that Snap7 files exist:
```bash
ls -la S7Server/snap7/
# Should show: snap7.h  libsnap7.so
```

## More Information

- Complete build guide: [BUILD_UBUNTU.md](BUILD_UBUNTU.md)
- Implementation details: [UBUNTU_IMPLEMENTATION.md](UBUNTU_IMPLEMENTATION.md)
- General information: [README.md](../README.md)

## All Make Commands

```bash
make         # Build in release mode
make debug   # Build in debug mode
make clean   # Clean build artifacts
make test    # Test the build
make install # Install system-wide (requires sudo)
make help    # Show all targets
```
