# Quick Start: Testing the 20-Variable Limit

This guide will help you quickly test whether the 20-variable limit exists in Snap7.

## Step 1: Build the Projects

### Option A: Using Visual Studio
1. Close and reopen `S7Server.sln` to refresh the solution
2. Right-click on the solution ? "Build Solution" (or press F7)
3. Both S7Server and S7Client should build successfully

### Option B: Using MSBuild (Command Line)
```powershell
msbuild S7Server.sln /p:Configuration=Release /p:Platform=x64
```

## Step 2: Prepare the Server

The server needs to have enough data in DB101 to support reading 50 REAL values.

### Check/Update address.csv
Ensure your `S7Server\address.csv` has at least 50 entries in DB101. The current configuration already includes DB101 with various offsets.

**Note:** The test client reads from **DB101** (not DB1) to match your actual configuration. It will read:
- DB101.REAL0, DB101.REAL4, DB101.REAL8, ..., DB101.REAL196

Each REAL takes 4 bytes, so:
- REAL0 = offset 0
- REAL1 = offset 4
- REAL2 = offset 8
- ...
- REAL49 = offset 196

The server will allocate DB101 with sufficient size based on your address.csv entries.

## Step 3: Start the Server

### As Administrator (for port 102):
```powershell
# Navigate to the output directory
cd x64\Release

# Run the server as Administrator
.\S7Server.exe
```

You should see:
```
========================================
S7 Server ISO-on-TCP (Snap7)
For Node-RED Testing
========================================

*** Server started successfully! ***

Server is running. Press Ctrl+C to stop.
```

## Step 4: Run the Client Test

Open a **second terminal/command prompt**:

```powershell
# Navigate to the output directory
cd x64\Release

# Run the client (doesn't need admin rights)
.\S7Client.exe
```

## Step 5: Interpret Results

### Scenario 1: NO Limit (Expected with Snap7)
```
========================================
Test Summary:
========================================
20 variables: 20/20 [PASS]
30 variables: 30/30 [PASS]
50 variables: 50/50 [PASS]
========================================

CONCLUSION: No 20 variable limit detected in Snap7!
```

**Interpretation**: The limit is likely in Node-RED's implementation, not in Snap7.

### Scenario 2: 20-Variable Limit Exists
```
========================================
Test Summary:
========================================
20 variables: 20/20 [PASS]
30 variables: 20/30 [FAIL]
50 variables: 20/50 [FAIL]
========================================

CONCLUSION: 20 variable limit detected!
```

**Interpretation**: Snap7 has an inherent limit. This would be unexpected.

### Scenario 3: Other Issues
```
========================================
Test Summary:
========================================
20 variables: 15/20 [FAIL]
30 variables: 15/30 [FAIL]
50 variables: 15/50 [FAIL]
========================================

CONCLUSION: Results are inconclusive. Check server configuration.
```

**Possible causes**:
- Server doesn't have enough data allocated in DB101
- PDU size too small
- Network issues
- Server crashed during test

## Troubleshooting

### "Failed to start server: Address already in use"
Port 102 is already in use. Options:
1. Stop the other application using port 102
2. Use a custom port (edit main.cpp and uncomment lines 396-398)

### "Connection failed: Connection refused"
- Server isn't running
- Firewall is blocking the connection
- Wrong IP address

### "DB1 not accessible"
- The CSV file wasn't loaded correctly
- DB101 wasn't created or registered properly
- Check the server console for allocation messages

### Client shows all reads as [FAIL]
- Server stopped/crashed
- DB101 doesn't exist or is too small
- Check server console for errors during client connection

## Advanced Testing

### Test with Custom Port (No Admin Required)

**Server** - Edit `S7Server\main.cpp`:
```cpp
// Around line 396, uncomment these lines:
int customPort = 10102;
Srv_SetParam(S7Server, p_u16_LocalPort, &customPort);
```

Rebuild and run (no admin needed):
```powershell
.\S7Server.exe
```

**Client**:
```powershell
.\S7Client.exe 127.0.0.1 0 0 10102
```

### Test with Remote Server

If you have a real PLC or S7 server on your network:

```powershell
.\S7Client.exe 192.168.1.100 0 1 102
```

(Adjust IP, rack, and slot as needed)

## Next Steps

Based on the test results:

1. **If NO limit in Snap7**: Investigate Node-RED's S7 node implementation
2. **If limit EXISTS**: Document the finding and explore workarounds
3. **If inconclusive**: Debug server configuration and retry

## Additional Information

- Server logs: Check the console output from S7Server.exe
- Client verbose output: All read operations show individual success/failure
- Performance comparison: Note the time difference between individual and multi-reads
