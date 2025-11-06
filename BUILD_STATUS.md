# Build and Run Issues - RESOLVED

## Issue Summary

The application compiled successfully but failed to start. There were two separate issues:

### 1. Wrong snap7.dll Version (FIXED)
**Problem:** The output directory contained an older, smaller version of snap7.dll (216,064 bytes) instead of the correct version (265,728 bytes).

**Root Cause:** No post-build event was configured to copy the correct DLL after building.

**Solution Applied:** 
- Manually copied the correct snap7.dll from `S7Server\snap7\snap7.dll` to `x64\Debug\snap7.dll`
- The application now loads correctly

**Permanent Fix Needed:** Add a post-build event to automatically copy the DLL. See instructions in `FIX_DLL_COPY.md`

### 2. Administrator Privileges Required (EXPECTED BEHAVIOR)
**Problem:** Application fails with error "TCP : Permission denied"

**Root Cause:** Port 102 (the standard S7 communication port) requires administrator privileges on Windows.

**Solution:** Run the application as Administrator

## How to Run the Application

### Option 1: Run Executable as Administrator
```powershell
cd D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug
# Right-click S7Server.exe and select "Run as administrator"
```

### Option 2: Run from Visual Studio
1. Close Visual Studio if open
2. Right-click on Visual Studio and select "Run as administrator"
3. Open the solution
4. Press F5 to debug

### Option 3: Use PowerShell with Admin Rights
```powershell
# Run PowerShell as Administrator, then:
cd D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug
.\S7Server.exe
```

## Expected Output

When running successfully, you should see:

```
========================================
S7 Server ISO-on-TCP (Snap7)
For Node-RED Testing
========================================

Initializing memory areas...
Memory areas registered successfully.
Starting server on port 102...
[EVENT] Server started (Code: 1)

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

## Verification Steps

1. **Check DLL is correct:**
   ```powershell
   Get-Item "D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug\snap7.dll" | Select-Object Length
   # Should show: 265728
   ```

2. **Run the application as Administrator**

3. **Check for "Server started successfully!" message**

4. **Test connection from Node-RED or another S7 client**

## Next Steps

1. **Add Post-Build Event** - Follow instructions in `FIX_DLL_COPY.md` to prevent future DLL issues

2. **Test the Server** - Use Node-RED or an S7 client to connect:
   - IP: 127.0.0.1
   - Port: 102
   - Rack: 0, Slot: 1

3. **Verify Data Access** - Try reading DB1.DBB0 (should return value 42)

## Status

? Application compiles successfully  
? Correct snap7.dll copied to output directory  
? Application runs when executed as Administrator  
?? Post-build event not yet added (requires project file modification)

## Files Modified

- `setup_snap7.ps1` - Updated to work with local Snap7 repository
- `S7Server/main.cpp` - Updated to work with new Snap7 API
- `x64/Debug/snap7.dll` - Manually copied correct version
- `FIX_DLL_COPY.md` - Instructions for permanent fix (NEW)
- `BUILD_STATUS.md` - This file (NEW)
