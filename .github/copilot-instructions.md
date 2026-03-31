# S7 Server ISO-on-TCP - Copilot Instructions

## Project Overview

A Siemens S7 compatible server implementation using the Snap7 library and ISO-on-TCP protocol. Designed for testing Node-RED S7 applications in controlled development environments. **Security Notice:** Testing/development only - not production-ready.

## Build & Run

### Windows (Visual Studio)
```bash
# Build
- Open S7Server.sln in Visual Studio 2022
- Select Release|x64 configuration
- Press F7 to build

# Run (requires Administrator)
cd x64\Release
.\S7Server.exe  # Right-click → "Run as administrator"
```

### Linux (CMake/Make)
```bash
# Build with CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Or use Makefile
make release

# Run (requires sudo for port 102)
sudo ./build/S7Server
```

### Testing
- **Manual Testing**: Start server, connect from Node-RED (node-red-contrib-s7) to 127.0.0.1:102
- **S7Client Test Tool**: Build and run `S7Client` project to test variable read limits and multi-variable operations
- No automated test suite exists; testing is done manually via Node-RED or the S7Client

## Architecture

### Core Components

1. **S7Server** (`S7Server/main.cpp`)
   - Primary server executable using Snap7 library
   - CSV-driven dynamic memory configuration from `address.csv`
   - Automatic value updates in sawtooth pattern (min→max→min) based on cycletime
   - Event callbacks for read/write/connection monitoring
   - Supports multiple memory areas: Data Blocks (DB), Inputs (I), Outputs (Q), Flags/Merkers (M), Timers (T), Counters (C)

2. **S7Client** (`S7Client/main.cpp`)
   - Test client for verifying variable read operations
   - Tests Snap7 multi-variable read limits (5/20/30/50 variable batches)
   - Validates server functionality without Node-RED

3. **CSV Configuration System** (`address.csv`)
   - Runtime memory initialization - no recompilation needed
   - Format: `tag,min,max,echelon,cycletime`
   - Tag format: `"DB<number>,<TYPE><offset>"` (e.g., `"DB101,REAL184"`)
   - Supported types: REAL (4 bytes), DWORD (4 bytes), INT (2 bytes), BOOL (1 bit)
   - Server auto-sizes Data Blocks based on highest offset + data type size

### Memory Areas

| Area | Type | Default Size | Description |
|------|------|--------------|-------------|
| DB (Data Blocks) | Dynamic | CSV-configured | Main data storage, sized per CSV config |
| I (Inputs) | Static | 256 bytes | Process input image |
| Q (Outputs) | Static | 256 bytes | Process output image |
| M (Merkers/Flags) | Static | 256 bytes | Flag memory |
| T (Timers) | Static | 512 bytes | Timer area |
| C (Counters) | Static | 512 bytes | Counter area |

**Note:** Data Blocks are dynamically allocated. Default `address.csv` configures 36 DBs (DB2, DB101-105, DB151-155, DB201-205, DB251-255, DB301-309, DB352-358) with 57 REAL values.

### Dynamic Value Updates

- Update thread runs every 100ms checking tag cycletimes
- Each tag updates independently based on its cycletime configuration
- Values follow sawtooth pattern: increment by `echelon` until `max`, then decrement to `min`
- Example: `min=0, max=100, echelon=0.5, cycletime=2000` → value changes by 0.5 every 2 seconds

## Key Conventions

### Memory Management
- **Zero-initialization mandatory**: All memory areas are zero-initialized with `memset` before registration
- **Cleanup on all paths**: Every allocation has corresponding cleanup in shutdown handlers and error paths
- **Snap7 registration pattern**:
  ```cpp
  byte* area = new byte[size];
  memset(area, 0, size);  // ALWAYS zero-initialize
  int result = Srv_RegisterArea(S7Server, areaType, index, area, size);
  if (result != 0) {
      delete[] area;  // Cleanup on failure
      // handle error
  }
  ```

### CSV Parsing & Configuration
- **CSV structure**: Quoted tags, comma-separated fields, header row required
- **Tag parsing**: Must extract DB number, data type, and offset from tag string (format: `"DB<num>,<TYPE><offset>"`)
- **Error handling**: Continue loading other entries on parse errors, log warnings
- **File location**: CSV must be in same directory as executable
- When adding new data types, update `DataType` enum and add parsing logic in `parseTag()` function

### Event Callbacks
- Three callback types: General events, Read events, Write events
- Callbacks use function pointer pattern: `void S7API CallbackName(void* usrPtr, PSrvEvent PEvent, int Size)`
- Log format: `[EVENT_TYPE] description` (e.g., `[CLIENT CONNECT] Client connected from IP`)
- Update callbacks when adding new event types or memory areas

### Platform-Specific Code
- **Port 102 requirement**: Requires Administrator (Windows) or sudo (Linux) privileges
- **Path separators**: Use Windows paths (`\`) in Visual Studio projects, Unix paths (`/`) in CMake/Makefiles
- **DLL/SO handling**: Windows needs `snap7.dll` in executable directory; Linux uses RPATH or LD_LIBRARY_PATH
- **Signal handling**: Use `<csignal>` for cross-platform SIGINT/SIGTERM handling

### Build Systems
- **Windows**: Visual Studio 2022 (`.sln`/`.vcxproj`) with x64 architecture only
- **Linux**: CMake (3.10+) or Makefile for builds
- **Snap7 dependency**: Not included in repo - users download separately
  - Header: `snap7.h` from `release/Wrappers/c-cpp/`
  - Windows: `snap7.lib` + `snap7.dll` from `build/bin/win64/`
  - Linux: `libsnap7.a` or `libsnap7.so` from build
- **Post-build**: Copy `address.csv` and `snap7.dll` to output directory automatically

### Node-RED Integration
- Primary testing/usage method
- Connection settings: `127.0.0.1:102`, Rack=0, Slot=1, PLC Type=S7-1200/S7-1500
- Variable format in Node-RED: `DB<num>,<TYPE><offset>` (e.g., `DB101,REAL184`)
- Docker setup available via `docker-compose.yml` with host access through `host.docker.internal`

## Common Operations

### Adding a New Data Type to CSV System
1. Add enum to `DataType` (e.g., `WORD`, `DINT`)
2. Define size constant (e.g., `const int WORD_SIZE = 2;`)
3. Update `parseTag()` function to recognize new type prefix
4. Update `parseDataType()` to return correct size
5. Add value conversion logic in dynamic update thread if needed
6. Document in README.md

### Adding a New Static Memory Area
1. Allocate: `byte* area = new byte[size];`
2. Zero-initialize: `memset(area, 0, size);`
3. Register: `Srv_RegisterArea(S7Server, areaType, index, area, size);`
4. Add to cleanup in `cleanup()` function
5. Update configuration output in startup logs
6. Document in README.md memory areas table

### Modifying Dynamic Update Behavior
- Update logic is in the background thread started in `main()`
- Modify `TagState` structure to track new state variables
- Update the while loop that checks `lastUpdate` and `cycletime`
- Ensure thread-safe access to shared memory areas (currently not locked - single writer model)

### Debugging Connection Issues
1. Check Administrator/sudo privileges (port 102 requirement)
2. Verify Snap7 DLL/SO is in correct location: `ldd S7Server` (Linux) or dependency walker (Windows)
3. Enable verbose logging in callbacks (already enabled by default)
4. Check Windows Firewall rules: `New-NetFirewallRule -DisplayName "S7 Server" -Direction Inbound -LocalPort 102 -Protocol TCP -Action Allow`
5. Test with S7Client before Node-RED to isolate issues
6. Monitor server console for connection events and read/write logs

## File Structure

```
S7-Server-Iso-on-tcp/
├── S7Server/                    # Main server implementation
│   ├── main.cpp                # Server code with CSV parsing & dynamic updates
│   ├── address.csv             # Runtime memory configuration
│   ├── snap7/                  # Snap7 library (not in repo - user provides)
│   │   ├── snap7.h
│   │   ├── snap7.lib/.a        # Platform-specific
│   │   └── snap7.dll/.so       # Platform-specific
│   └── S7Server.vcxproj        # Visual Studio project
├── S7Client/                    # Test client for validation
│   ├── main.cpp                # Client test code
│   ├── README.md               # Client usage docs
│   └── S7Client.vcxproj        # Visual Studio project
├── S7Server.sln                # Visual Studio solution
├── CMakeLists.txt              # Linux build configuration
├── Makefile                    # Linux alternative build
├── address.csv                 # Default CSV config (root copy)
├── docker-compose.yml          # Node-RED test environment
├── start-environment.ps1       # Automated server + Node-RED launcher
├── stop-environment.ps1        # Environment shutdown
├── setup_snap7.ps1/.bat        # Windows Snap7 installation helper
├── setup_snap7_linux.sh        # Linux Snap7 installation helper
└── build_ubuntu.sh             # Ubuntu build script
```

## Security Considerations

- **No authentication**: S7 protocol has no built-in authentication mechanism
- **No encryption**: All data transmitted in plaintext
- **Arbitrary read/write**: Any connected client can read/write any memory area
- **DoS vulnerability**: No connection limits or rate limiting
- **Port 102 requirement**: Requires elevated privileges, expanding attack surface
- **Production use**: Explicitly NOT recommended - testing/development only

See `doc/SECURITY.md` for full security analysis.

## References

- [Snap7 GitHub](https://github.com/davenardella/snap7) - Official source (recommended)
- [Snap7 Server API](http://snap7.sourceforge.net/snap7_server.html) - Server function reference
- [Node-RED S7 Contrib](https://flows.nodered.org/node/node-red-contrib-s7) - Node-RED integration
- See README.md for complete setup instructions
- See QUICKSTART.md for 5-minute setup guide
- See CONTRIBUTING.md for development guidelines
