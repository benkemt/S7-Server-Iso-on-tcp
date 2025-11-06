# Find What's Using Port 102

## Problem
Port 102 is already in use (`LISTENING`), but your S7Server fails to start. This means **another application is already using port 102**.

## Find the Application Using Port 102

### Method 1: Using netstat (Detailed)

Run this in **PowerShell as Administrator**:

```powershell
netstat -ano | findstr :102
```

Example output:
```
TCP    0.0.0.0:102    0.0.0.0:0   LISTENING       1234
```

The last number (`1234`) is the **Process ID (PID)**.

### Method 2: Get Process Details

Once you have the PID, find the application name:

```powershell
# Replace 1234 with the actual PID from step 1
Get-Process -Id 1234 | Select-Object Id, ProcessName, Path
```

Or use this one-liner to see everything at once:

```powershell
netstat -ano | findstr :102 | ForEach-Object {
    $pid = $_.Split(' ')[-1]
    Get-Process -Id $pid | Select-Object Id, ProcessName, Path
}
```

### Method 3: Using TCPView (GUI Tool - Easiest)

1. Download **TCPView** from Microsoft Sysinternals:
   - https://learn.microsoft.com/en-us/sysinternals/downloads/tcpview
2. Run **TCPView** as Administrator
3. Look for port **102** in the "Local Port" column
4. You'll see the process name and PID

### Method 4: Using PowerShell (All-in-One)

```powershell
Get-NetTCPConnection -LocalPort 102 | Select-Object LocalAddress, LocalPort, State, OwningProcess | ForEach-Object {
    $process = Get-Process -Id $_.OwningProcess
    [PSCustomObject]@{
      Port = $_.LocalPort
        State = $_.State
        PID = $_.OwningProcess
        ProcessName = $process.ProcessName
   Path = $process.Path
    }
}
```

## Common Culprits

Applications that commonly use port 102:

1. **Another instance of your S7Server** - Check if it's already running
2. **Siemens S7 software** (STEP 7, TIA Portal, PLCSIM)
3. **Other PLC simulators** (Snap7 examples, factory I/O)
4. **PLCsim Advanced**
5. **Other industrial automation software**

## Solutions

### Solution 1: Stop the Other Application

Once you identify the process:

```powershell
# Replace 1234 with the actual PID
Stop-Process -Id 1234 -Force
```

Or close it normally from Task Manager:
1. Open **Task Manager** (Ctrl+Shift+Esc)
2. Go to **Details** tab
3. Find the process by PID
4. Right-click ? **End Task**

### Solution 2: Kill All Processes on Port 102

**?? WARNING: This will forcefully close the application!**

```powershell
# Run as Administrator
$connections = Get-NetTCPConnection -LocalPort 102 -ErrorAction SilentlyContinue
foreach ($conn in $connections) {
    Stop-Process -Id $conn.OwningProcess -Force
    Write-Host "Killed process $($conn.OwningProcess)"
}
```

### Solution 3: Use a Different Port

If you can't stop the other application, configure your S7Server to use a different port.

Edit `S7Server/main.cpp` and uncomment these lines (around line 121):

```cpp
// Configure server port (optional: use non-privileged port for testing)
int customPort = 10102;  // Uncomment this
Srv_SetParam(S7Server, p_u16_LocalPort, &customPort);  // Uncomment this
std::cout << "NOTE: Using custom port 10102 (no admin privileges required)" << std::endl;  // Uncomment this
```

Then rebuild and run your server - it will use port 10102 instead.

**Remember:** When using a custom port, you must configure Node-RED to use that port too!

### Solution 4: Check for S7Server Already Running

Maybe your server IS running and you don't realize it:

```powershell
Get-Process | Where-Object {$_.ProcessName -like "*S7Server*"} | Select-Object Id, ProcessName, Path
```

If you find it:
```powershell
# Kill your own S7Server instances
Get-Process -Name "S7Server" | Stop-Process -Force
```

## Quick Diagnostic Script

Save this as `check_port_102.ps1` and run as Administrator:

```powershell
Write-Host "=== Checking Port 102 ===" -ForegroundColor Cyan
Write-Host ""

# Check if port is in use
$connections = Get-NetTCPConnection -LocalPort 102 -ErrorAction SilentlyContinue

if ($connections) {
    Write-Host "Port 102 is IN USE by:" -ForegroundColor Yellow
    Write-Host ""
    
    foreach ($conn in $connections) {
    $process = Get-Process -Id $conn.OwningProcess -ErrorAction SilentlyContinue
        if ($process) {
          Write-Host "  PID: $($conn.OwningProcess)" -ForegroundColor White
        Write-Host "  Process: $($process.ProcessName)" -ForegroundColor White
     Write-Host "  Path: $($process.Path)" -ForegroundColor Gray
            Write-Host "  State: $($conn.State)" -ForegroundColor White
          Write-Host ""
            
       $choice = Read-Host "Do you want to kill this process? (y/N)"
 if ($choice -eq "y" -or $choice -eq "Y") {
          Stop-Process -Id $conn.OwningProcess -Force
  Write-Host "  Process killed!" -ForegroundColor Green
}
        }
    }
} else {
    Write-Host "Port 102 is FREE - Your S7Server should be able to start!" -ForegroundColor Green
}

Write-Host ""
Write-Host "=== Done ===" -ForegroundColor Cyan
```

Run it:
```powershell
.\check_port_102.ps1
```

## Verify Port is Free

After stopping the other application:

```powershell
netstat -an | findstr :102
```

If you get **no output**, the port is free and your S7Server should start successfully!

## Prevention

To avoid this in the future:

1. **Always check** if S7Server is already running before starting it again
2. **Close other S7/PLC software** before starting your server
3. **Use a custom port** (like 10102) for development/testing
4. **Add error handling** in your server startup code

## Still Having Issues?

If you can't identify or stop the process:

1. **Restart your computer** - This will free all ports
2. **Check Windows Services** - Some applications run as services:
   ```powershell
   Get-Service | Where-Object {$_.Status -eq "Running"} | Select-Object Name, DisplayName
   ```
3. **Use port 10102** for your server instead (see Solution 3 above)

---

## Next Steps

Once port 102 is free:
1. Run your S7Server as Administrator
2. Verify it starts with "Server started successfully!"
3. Check with: `netstat -an | findstr :102`
4. Connect from Node-RED

Need help? Check the main documentation files in your project.
