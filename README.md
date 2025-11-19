# S7-Server-Iso-on-tcp
A Siemens S7 server implementation using ISO-on-TCP protocol with the Snap7 library. This server is designed for testing Node-RED S7 applications on Windows and Linux.

## ⚠️ Security Notice

**This server is intended for testing and development purposes only.** It should be run in isolated, trusted networks. See [SECURITY.md](doc/SECURITY.md) for detailed security considerations.

## Features

- **ISO-on-TCP Protocol**: Full implementation of Siemens S7 communication protocol
- **CSV-Based Configuration**: Dynamic memory initialization using CSV files - no code changes needed
- **Dynamic Tag Value Updates**: Automatic value changes based on configurable cycle times and step increments
- **Multiple Memory Areas**: Supports Data Blocks (DB), Inputs (I), Outputs (Q), Flags (M), Timers (T), and Counters (C)
- **Real-time Monitoring**: Event callbacks for server operations, read/write operations
- **Flexible Configuration**: Easy to customize memory layout and test values via CSV file
- **Windows Compatible**: Built with Visual Studio 2022 for Windows systems

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
- Snap7 library (download and build instructions in [BUILD_UBUNTU.md](doc/BUILD_UBUNTU.md))
- Root/sudo privileges (required for port 102)

## Memory Configuration

### CSV-Based Dynamic Configuration

The server now uses a CSV-based configuration file ('address.csv') to dynamically initialize Data Blocks. This allows you to modify the server's memory layout without changing code or recompiling.

#### CSV Format

The configuration file uses the following format:

```csv
tag,min,max,echelon,cycletime
"DB101,REAL184",0,1800,0.5,2000
"DB101,REAL14",0,1800,0.5,2000
"DB151,REAL14",0,100,1,2000
```

#### CSV Fields

| Field | Description |
|-------|-------------|
| **tag** | S7 address in format: `DB<number>,REAL<offset>` where REAL indicates a 4-byte float at the specified byte offset |
| **min** | Minimum value for the tag (starting value and lower boundary) |
| **max** | Maximum value for the tag (upper boundary) |
| **echelon** | Step/increment value used for dynamic value updates |
| **cycletime** | Update interval in milliseconds - determines how often the tag value changes |

#### Example Configuration

The default 'address.csv' file configures 36 Data Blocks with 57 REAL values:

- **DB101-DB105**: General purpose blocks with ranges 0-1800
- **DB151-DB155**: Configuration blocks with range 0-100
- **DB201-DB205**: Small value blocks with range 0-20
- **DB251-DB255**: Percentage blocks with range 0-100
- **DB301-DB309**: Process blocks with various ranges, including negative values (-50 to 50)
- **DB352-DB358**: Status blocks with range 0-100

Data Blocks are automatically sized based on the highest offset + 4 bytes (REAL size) defined in the CSV.

#### Dynamic Value Updates

The server includes an automatic value update feature that simulates changing process values:

- **Update Mechanism**: Every 100ms, the server checks which tags need updating based on their individual `cycletime` settings
- **Value Pattern**: Each tag value follows a sawtooth pattern:
  1. Starts at the `min` value
  2. Increments by `echelon` every `cycletime` milliseconds
  3. When `max` is reached, switches to decrementing
  4. When `min` is reached, switches back to incrementing
- **Independent Timing**: Each tag updates independently according to its own `cycletime`
- **Example**: A tag with `min=0`, `max=100`, `echelon=0.5`, and `cycletime=2000` will:
  - Start at 0
  - Increase by 0.5 every 2 seconds
  - Reach 100 after 400 seconds (200 updates)
  - Then decrease back to 0 at the same rate

This feature is ideal for testing applications that need to monitor changing values, such as temperature sensors, flow meters, or other process variables.

### Standard Memory Areas

In addition to the dynamically configured Data Blocks, the server provides:

| Area | Type | Size | Description |
|------|------|------|-------------|
| I | Inputs | 256 bytes | Process input image |
| Q | Outputs | 256 bytes | Process output image |
| M | Flags/Merkers | 256 bytes | Flag memory |
| T | Timers | 512 bytes | Timer area |
| C | Counters | 512 bytes | Counter area |

### Customizing Configuration

To customize the server configuration:

1. Edit the 'address.csv' file in the project root directory
2. Add or modify entries following the CSV format
3. Restart the server - changes are loaded automatically on startup

**Note**: The CSV file must be in the same directory as the server executable when running.

## Setup Instructions

> 📋 **Choose Your Platform:**
> - **Windows**: Follow the instructions below
> - **Ubuntu/Linux**: See [BUILD_UBUNTU.md](doc/BUILD_UBUNTU.md) for detailed Linux build instructions

### Windows Setup

### 1. Download Snap7 Library

**Note:** The SourceForge download site does not have the latest version of Snap7. We recommend cloning from the official GitHub repository:

**Option A: Clone from GitHub (Recommended)**
1. Clone the Snap7 repository:
   ```bash
   git clone https://github.com/davenardella/snap7.git
   ```
2. Build the library following the instructions in the repository
3. Locate the built files for Windows x64

**Option B: Download from SourceForge (Older Version)**
1. Visit http://snap7.sourceforge.net/
2. Download an older Snap7 release for Windows
3. Extract the archive

### 2. Install Snap7 Files

From the built or extracted Snap7 files, copy the following to the `S7Server/snap7/` directory in this project:

```
S7Server/snap7/
├── snap7.h        (from snap7/release/Wrappers/c-cpp/)
├── snap7.lib    (from snap7/build/bin/x64/ or snap7/build/bin/win64/)
└── snap7.dll    (from snap7/build/bin/x64/ or snap7/build/bin/win64/)
```

**Note**: Use the x64 (64-bit) version of the library files.

Example PowerShell commands (adjust paths based on where you built/extracted Snap7):
```powershell
# Create snap7 directory
New-Item -ItemType Directory -Force -Path S7Server\snap7

# Copy files from GitHub build (adjust path to your Snap7 clone location)
Copy-Item "C:\path\to\snap7\release\Wrappers\c-cpp\snap7.h" -Destination "S7Server\snap7\"
Copy-Item "C:\path\to\snap7\build\bin\win64\snap7.lib" -Destination "S7Server\snap7\"
Copy-Item "C:\path\to\snap7\build\bin\win64\snap7.dll" -Destination "S7Server\snap7\"
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

### 4. Copy Configuration File

Before running the server, ensure the 'address.csv' file is in the same directory as the executable:

```powershell
# For Debug build
Copy-Item "address.csv" -Destination "x64\Debug\"

# For Release build
Copy-Item "address.csv" -Destination "x64\Release\"
```

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

Loading CSV configuration from 'dresse.csv'...
Loaded 57 entries from CSV configuration.

Initializing Data Blocks from CSV configuration...
Allocated DB2: 452 bytes
Allocated DB101: 188 bytes
Allocated DB102: 188 bytes
[... additional DBs ...]
  DB101.REAL184 = 0 (range: 0 to 1800)
  DB101.REAL14 = 0 (range: 0 to 1800)
[... additional initializations ...]

Initializing memory areas...
Memory areas registered successfully.
Starting server on port 102...

*** Server started successfully! ***

Initialized 57 tag states for dynamic updates.
Dynamic tag value updates enabled with 100ms update interval.

========================================
S7 Server Configuration:
========================================
Protocol: ISO-on-TCP
Port: 102
Data Blocks:
  - DB2: 452 bytes
  - DB101: 188 bytes
  - DB102: 188 bytes
  - DB103: 188 bytes
  [... additional DBs ...]
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

The following examples use the default 'address.csv' configuration:

**Read REAL values from DB101:**
- Variable: `DB101,REAL184` (reads float at byte 184, range 0-1800)
- Variable: `DB101,REAL14` (reads float at byte 14, range 0-1800)

**Read REAL values from DB151:**
- Variable: `DB151,REAL14` (reads float at byte 14, range 0-100)

**Read REAL values from DB201:**
- Variable: `DB201,REAL184` (reads float at byte 184, range 0-20)
- Variable: `DB201,REAL14` (reads float at byte 14, range 0-20)

**Read REAL values from DB301 (including negative values):**
- Variable: `DB301,REAL112` (reads float at byte 112, range -50 to 50)
- Variable: `DB301,REAL14` (reads float at byte 14, range -50 to 50)
- Variable: `DB301,REAL116` (reads float at byte 116, range 0-100)

**Read from DB2 (configured in CSV):**
- Variable: `DB2,REAL448` (reads float at byte 448, range 0-1400)

**Write to any Data Block:**
- Variable: `DB101,REAL184` with floating-point value (e.g., 1234.5)
- Variable: `DB301,REAL14` with value in range -50 to 50
- Any value you write will persist until server restart

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

- [Snap7 GitHub Repository](https://github.com/davenardella/snap7) - Official source code (recommended)
- [Snap7 Official Documentation](http://snap7.sourceforge.net/)
- [Snap7 Server Class Reference](http://snap7.sourceforge.net/snap7_server.html)
- [Node-RED S7 Contrib](https://flows.nodered.org/node/node-red-contrib-s7)
- [Siemens S7 Protocol](https://en.wikipedia.org/wiki/S7_communication)

## Support

For issues or questions:
1. Check the Troubleshooting section above
2. Review the Snap7 documentation
3. Open an issue in this repository
