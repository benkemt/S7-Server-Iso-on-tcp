# S7-Server-Iso-on-tcp
A Siemens S7 server implementation using ISO-on-TCP protocol with the Snap7 library. This server is designed for testing Node-RED S7 applications on Windows and Linux.

## ⚠️ Security Notice

**This server is intended for testing and development purposes only.** It should be run in isolated, trusted networks. See [SECURITY.md](SECURITY.md) for detailed security considerations.

## Features

- **ISO-on-TCP Protocol**: Full implementation of Siemens S7 communication protocol
- **Multiple Memory Areas**: Supports Data Blocks (DB), Inputs (I), Outputs (Q), Flags (M), Timers (T), and Counters (C)
- **Real-time Monitoring**: Event callbacks for server operations, read/write operations
- **Pre-configured Test Data**: Includes test data in Data Blocks for immediate testing
- **Cross-Platform**: Works on Windows (Visual Studio) and Linux (CMake/GCC)

## Requirements

### Windows
- Windows 10 or later
- Visual Studio 2022 (Community, Professional, or Enterprise)
- Snap7 library (download instructions below)
- Administrator privileges (required for port 102)

### Ubuntu/Linux
- Ubuntu 20.04 LTS or later (or any Debian-based distribution)
- CMake 3.10 or higher
- GCC/G++ compiler (build-essential package)
- Snap7 library (download and build instructions in [BUILD_UBUNTU.md](BUILD_UBUNTU.md))
- Root/sudo privileges (required for port 102)

## Memory Configuration

The server provides the following memory areas:

| Area | Type | Size | Description |
|------|------|------|-------------|
| DB1 | Data Block | 256 bytes | General purpose data block |
| DB2 | Data Block | 512 bytes | Extended data block |
| DB3 | Data Block | 128 bytes | Test data block |
| I | Inputs | 256 bytes | Process input image |
| Q | Outputs | 256 bytes | Process output image |
| M | Flags/Merkers | 256 bytes | Flag memory |
| T | Timers | 512 bytes | Timer area |
| C | Counters | 512 bytes | Counter area |

### Test Data

The server initializes with the following test data:

- **DB1.DBB0**: 42
- **DB1.DBB1**: 100
- **DB1.DBB2**: 255 (0xFF)
- **DB2.DBB0**: 1
- **DB2.DBB1**: 2
- **DB2.DBB2**: 3

## Setup Instructions

> 📋 **Choose Your Platform:**
> - **Windows**: Follow the instructions below
> - **Ubuntu/Linux**: See [BUILD_UBUNTU.md](BUILD_UBUNTU.md) for detailed Linux build instructions

### Windows Setup

### 1. Download Snap7 Library

1. Visit the [Snap7 official website](http://snap7.sourceforge.net/)
2. Download the latest Snap7 release for Windows
3. Extract the archive

### 2. Install Snap7 Files

From the extracted Snap7 archive, copy the following files to the `S7Server/snap7/` directory in this project:

```
S7Server/snap7/
├── snap7.h        (from snap7-full-x.x.x/include/)
├── snap7.lib      (from snap7-full-x.x.x/build/bin/x64/)
└── snap7.dll      (from snap7-full-x.x.x/build/bin/x64/)
```

**Note**: Use the x64 (64-bit) version of the library files.

Example PowerShell commands:
```powershell
# Create snap7 directory
New-Item -ItemType Directory -Force -Path S7Server\snap7

# Copy files (adjust paths to your Snap7 download location)
Copy-Item "C:\path\to\snap7-full-x.x.x\include\snap7.h" -Destination "S7Server\snap7\"
Copy-Item "C:\path\to\snap7-full-x.x.x\build\bin\x64\snap7.lib" -Destination "S7Server\snap7\"
Copy-Item "C:\path\to\snap7-full-x.x.x\build\bin\x64\snap7.dll" -Destination "S7Server\snap7\"
```

### 3. Build the Project

1. Open `S7Server.sln` in Visual Studio 2022
2. Select the build configuration:
   - **Debug|x64** for development
   - **Release|x64** for production
3. Build the solution (F7 or Build → Build Solution)

The compiled executable will be in:
- Debug: `x64/Debug/S7Server.exe`
- Release: `x64/Release/S7Server.exe`

## Running the Server

### Important: Administrator Privileges

The S7 protocol uses TCP port 102, which requires administrator privileges on Windows.

**To run the server:**

1. Right-click on `S7Server.exe`
2. Select **"Run as administrator"**

Or from an elevated Command Prompt/PowerShell:
```cmd
cd x64\Release
S7Server.exe
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

## Testing with Node-RED

### Install Node-RED S7 Node

```bash
npm install node-red-contrib-s7
```

### Configure S7 Connection in Node-RED

1. Add an S7 input or output node to your flow
2. Configure the connection:
   - **PLC**: S7-1200 or S7-1500
   - **IP Address**: 127.0.0.1 (or your server's IP)
   - **Port**: 102
   - **Rack**: 0
   - **Slot**: 1
   - **Transport**: ISO-on-TCP

### Read/Write Examples

**Read from DB1:**
- Variable: `DB1,INT0` (reads integer at byte 0)
- Variable: `DB1,BYTE1` (reads byte at byte 1)
- Variable: `DB1,REAL4` (reads real/float at byte 4)

**Write to DB1:**
- Variable: `DB1,INT0` with value
- Variable: `DB1,BYTE2` with value

**Read Inputs/Outputs:**
- `I0.0` - Input bit 0.0
- `Q0.0` - Output bit 0.0
- `M0.0` - Flag bit 0.0

## Troubleshooting

### Error: Failed to start server

**Problem**: Port 102 access denied

**Solution**: Run the application as Administrator

### Error: Cannot find snap7.dll

**Problem**: The DLL is not in the correct location

**Solution**: 
- Ensure `snap7.dll` is in the same directory as `S7Server.exe`
- Or copy `snap7.dll` from `S7Server/snap7/` to the output directory

### Node-RED cannot connect

**Check**:
1. Server is running and shows "Server started successfully"
2. Windows Firewall is not blocking port 102
3. IP address and port are correct in Node-RED configuration
4. Server shows "Client connected" message when Node-RED tries to connect

### Add Firewall Rule (if needed)

```powershell
New-NetFirewallRule -DisplayName "S7 Server" -Direction Inbound -LocalPort 102 -Protocol TCP -Action Allow
```

## Development

### Project Structure

```
S7-Server-Iso-on-tcp/
├── S7Server.sln              # Visual Studio solution
├── S7Server/
│   ├── S7Server.vcxproj      # Visual Studio project
│   ├── main.cpp              # Server implementation
│   └── snap7/                # Snap7 library files (not included)
│       ├── snap7.h
│       ├── snap7.lib
│       └── snap7.dll
└── README.md                 # This file
```

### Modifying Memory Areas

To add or modify memory areas, edit `main.cpp`:

1. Allocate memory for your area
2. Initialize the memory if needed
3. Register the area with `Srv_RegisterArea()`

Example:
```cpp
// Add a new Data Block 10 with 1024 bytes
byte* DB10 = new byte[1024];
memset(DB10, 0, 1024);
Srv_RegisterArea(S7Server, srvAreaDB, 10, DB10, 1024);
```

### Event Callbacks

The server includes event callbacks for monitoring:

- **EventCallback**: General server events (start, stop, client connect/disconnect)
- **ReadEventCallback**: Logs read operations
- **WriteEventCallback**: Logs write operations

These can be customized in `main.cpp` to add custom logic.

## License

This project is provided under the MIT License. See the LICENSE file for details.

The Snap7 library is licensed under the GNU LGPL v3. See [Snap7 License](http://snap7.sourceforge.net/licensing.html) for more information.

## References

- [Snap7 Official Documentation](http://snap7.sourceforge.net/)
- [Snap7 Server Class Reference](http://snap7.sourceforge.net/snap7_server.html)
- [Node-RED S7 Contrib](https://flows.nodered.org/node/node-red-contrib-s7)
- [Siemens S7 Protocol](https://en.wikipedia.org/wiki/S7_communication)

## Support

For issues or questions:
1. Check the Troubleshooting section above
2. Review the Snap7 documentation
3. Open an issue in this repository
