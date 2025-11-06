# Fix: TCP Permission Denied Error

## Problem
You're getting the error: **"TCP : Permission denied"**

This happens because **port 102** (the standard S7 communication port) requires **administrator privileges** on Windows.

## Solution 1: Run as Administrator (RECOMMENDED)

### From Command Line:
```powershell
# Open PowerShell as Administrator, then:
cd D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug
.\S7Server.exe
```

### From File Explorer:
1. Navigate to: `D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug`
2. Find `S7Server.exe`
3. **Right-click** on it
4. Select **"Run as administrator"**
5. Click **"Yes"** on the UAC prompt

### From Visual Studio:
1. **Close Visual Studio** completely
2. Find Visual Studio 2022 in Start Menu
3. **Right-click** on "Visual Studio 2022"
4. Select **"Run as administrator"**
5. Open your solution: `S7Server.sln`
6. Press **F5** to run/debug

## Solution 2: Use a Non-Privileged Port (FOR TESTING ONLY)

If you can't or don't want to run as administrator, you can use a different port:

### Modify the Code:

In `S7Server/main.cpp`, find these lines (around line 118):
```cpp
// Configure server port (optional: use non-privileged port for testing)
// Uncomment the following lines to use port 10102 instead of 102 (no admin required)
// int customPort = 10102;
// Srv_SetParam(S7Server, p_u16_LocalPort, &customPort);
// std::cout << "NOTE: Using custom port 10102 (no admin privileges required)" << std::endl;
```

**Uncomment** those lines to make it:
```cpp
// Configure server port (optional: use non-privileged port for testing)
int customPort = 10102;
Srv_SetParam(S7Server, p_u16_LocalPort, &customPort);
std::cout << "NOTE: Using custom port 10102 (no admin privileges required)" << std::endl;
```

### Rebuild and Run:
1. Press **Ctrl+Shift+B** to rebuild
2. Run normally (no admin needed)

### Connect from Node-RED:
When using a custom port, configure your S7 node:
- **IP Address**: `127.0.0.1`
- **Port**: `10102` (instead of 102)
- **PLC Type**: S7-1200
- **Rack**: 0
- **Slot**: 1

## Verify It's Working

When running successfully as Administrator on port 102, you should see:

```
========================================
S7 Server ISO-on-TCP (Snap7)
For Node-RED Testing
========================================

Initializing memory areas...
Memory areas registered successfully.
Starting server on port 102...
NOTE: Port 102 requires administrator privileges!
[EVENT] Server started (Code: 1)

*** Server started successfully! ***

========================================
S7 Server Configuration:
========================================
Protocol: ISO-on-TCP
Port: 102
...
========================================

Server is running. Press Ctrl+C to stop.
```

## Why Port 102?

Port 102 is the **standard port** for Siemens S7 communication (ISO-on-TCP protocol). 

- **Ports 0-1023** are "well-known ports" and require administrator privileges
- **Ports 1024-65535** are user ports and don't require admin rights

## Troubleshooting

### Still getting permission denied?
- Make sure you're **really** running as Administrator
- Check if another application is using port 102: `netstat -an | findstr :102`
- Try restarting your computer
- Check Windows Firewall isn't blocking the application

### Can't run as Administrator?
- Use Solution 2 with a custom port (10102)
- Ask your system administrator for permissions

### Node-RED can't connect?
- Verify the server shows "Server started successfully!"
- Check you're using the correct IP and port in Node-RED
- If using custom port, make sure Node-RED is configured to use that port
- Check Windows Firewall rules

## Quick Test

After starting the server as Administrator, test the connection:

```powershell
# In another PowerShell window:
Test-NetConnection -ComputerName 127.0.0.1 -Port 102
```

You should see `TcpTestSucceeded : True`

## Next Steps

Once running:
1. ? Server should display "Server started successfully!"
2. ? Open Node-RED: `http://localhost:1880`
3. ? Add an S7 node and configure it
4. ? Try reading `DB1,INT0` (should return value `42`)

---

**Remember**: For production use with standard S7 tools and Node-RED, **port 102 is recommended**. Only use custom ports for testing without admin rights.
