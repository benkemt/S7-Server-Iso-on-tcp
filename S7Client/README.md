# S7 Client Test Application

This is a Snap7-based test client application designed to verify the 20-variable limit when reading from an S7 server.

## Purpose

This client application tests whether the Snap7 library itself has a 20-variable limit when using the `Cli_ReadMultiVars` function, or if the limitation exists elsewhere (e.g., in Node-RED's implementation).

The client reads from **DB101** to match the actual server configuration in `address.csv`.

## Features

- **Test 1**: Reads 5 variables individually using `Cli_ReadArea` (baseline)
- **Test 2**: Reads 20 variables using `Cli_ReadMultiVars` from DB101
- **Test 3**: Reads 30 variables using `Cli_ReadMultiVars` from DB101 (tests beyond 20 limit)
- **Test 4**: Reads 50 variables using `Cli_ReadMultiVars` from DB101 (stress test)

Each test measures execution time and reports success/failure for each variable read.

## Building

### Prerequisites
- Visual Studio 2022 (or compatible version with v143 toolset)
- Snap7 library (already included in `S7Server\snap7` directory)
- C++14 compiler support

### Build Steps

1. Open `S7Server.sln` in Visual Studio
2. Select the `S7Client` project
3. Build the project (Ctrl+Shift+B)
4. The executable will be in `x64\Debug\S7Client.exe` or `x64\Release\S7Client.exe`

The post-build event automatically copies `snap7.dll` to the output directory.

## Usage

### Basic Usage (Local Server)
```bash
S7Client.exe
```

### Custom Server IP
```bash
S7Client.exe 192.168.1.100
```

### Full Parameters
```bash
S7Client.exe <IP> <Rack> <Slot> <Port>
```

**Examples:**
```bash
# Connect to local server on default port 102
S7Client.exe 127.0.0.1 0 0 102

# Connect to remote server
S7Client.exe 192.168.1.100 0 0 102

# Connect to server on custom port (if testing without admin rights)
S7Client.exe 127.0.0.1 0 0 10102
```

## Testing Procedure

1. Start the S7 Server (`S7Server.exe`)
2. Ensure the server has at least DB1 with 200 bytes (50 REALs)
3. Run the client application
4. Review the test results

## Expected Results

### If NO 20-variable limit in Snap7:
- Test 2 (20 vars): 20/20 [PASS]
- Test 3 (30 vars): 30/30 [PASS]
- Test 4 (50 vars): 50/50 [PASS]
- Conclusion: "No 20 variable limit detected in Snap7!"

### If 20-variable limit EXISTS in Snap7:
- Test 2 (20 vars): 20/20 [PASS]
- Test 3 (30 vars): <30/30 [FAIL]
- Test 4 (50 vars): <50/50 [FAIL]
- Conclusion: "20 variable limit detected!"

## Implementation Details

### Data Conversion
The application includes helper functions to convert between:
- S7 REAL format (big-endian IEEE 754)
- Native float format (little-endian on Windows x86/x64)

### Snap7 Functions Used
- `Cli_Create()`: Creates client instance
- `Cli_ConnectTo()`: Establishes connection
- `Cli_ReadArea()`: Reads a single memory area
- `Cli_ReadMultiVars()`: Reads multiple variables in one request
- `Cli_Disconnect()`: Closes connection
- `Cli_Destroy()`: Frees client resources

## Troubleshooting

### Connection Failed
- Ensure S7Server is running
- Check firewall settings
- Verify IP address and port
- If using port 102, run both client and server as Administrator

### All Reads Fail
- Verify DB101 exists on the server
- Check that DB101 has sufficient size (minimum 200 bytes for all tests)
- Review server console for error messages

### Partial Success in Multi-Variable Tests
- Check PDU size negotiation in the connection info
- Verify server has enough memory allocated for DB101
- Look for specific error codes in failed reads

## Performance Notes

Reading variables individually is slower than using `Cli_ReadMultiVars`:
- Individual reads: ~5 network round-trips for 5 variables
- Multi-read: 1 network round-trip for up to 20 variables

The performance difference becomes more significant with larger variable counts.

## License

This test application uses the Snap7 library, which is licensed under LGPL.
