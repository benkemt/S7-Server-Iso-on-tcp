# Complete Setup Guide - All Options

## Choose Your Setup

### Option 1: Docker + Windows (Recommended) ??

**Best for:** Node-RED development, testing, easy reset

```powershell
# Run as Administrator
.\start-environment.ps1
```

- ? S7 Server runs on Windows (best performance)
- ? Node-RED runs in Docker (isolated, easy to manage)
- ? One command to start both
- ? Use `host.docker.internal:102` in Node-RED

**See:** `DOCKER_SUMMARY.md` for complete guide

---

### Option 2: Everything on Windows

**Best for:** Simple testing, no Docker needed

**Step 1:** Build the project
```powershell
# Open in Visual Studio 2022
# Build as Release|x64
```

**Step 2:** Install Node-RED globally
```powershell
npm install -g node-red
npm install -g node-red-contrib-s7
```

**Step 3:** Run both
```powershell
# Terminal 1 (as Administrator)
cd D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Release
.\S7Server.exe

# Terminal 2
node-red
```

**Step 4:** Configure Node-RED
- Open http://localhost:1880
- S7 node settings: `127.0.0.1:102`

---

### Option 3: Everything in Docker (Advanced)

**Best for:** Production deployment, Linux servers

*Not yet implemented - S7 Server needs Linux port*

---

## Quick Decision Tree

```
Do you have Docker Desktop?
?
?? Yes ? Use Option 1 (Docker + Windows)
?         ? Most flexible
?         ? Easy to reset
?      ? Good for development
?
?? No  ? Use Option 2 (Everything on Windows)
          ? Simplest setup
     ? No Docker needed
          ? Good for quick tests
```

## File Guide

| File | Purpose |
|------|---------|
| `QUICKSTART.md` | Basic setup (Option 2) |
| `DOCKER_SUMMARY.md` | Docker setup overview |
| `DOCKER_SETUP.md` | Docker details & troubleshooting |
| `DOCKER_QUICKREF.md` | Docker quick commands |
| `start-environment.ps1` | Auto-start script (Docker) |
| `stop-environment.ps1` | Auto-stop script (Docker) |
| `setup_snap7.ps1` | Setup Snap7 library |
| `check_port_102.ps1` | Check port usage |
| `FIX_PERMISSION_DENIED.md` | Port 102 admin fix |
| `FIND_PORT_102_USER.md` | Find what's using port 102 |

## Prerequisites

### For All Options
- ? Windows 10/11
- ? Visual Studio 2022
- ? Snap7 library (run `setup_snap7.ps1`)
- ? Administrator privileges

### Additional for Docker Option
- ? Docker Desktop for Windows
- ? WSL 2 enabled

### Additional for Windows-Only Option
- ? Node.js (v14 or later)
- ? npm

## Common Connection Settings

### Node-RED S7 Node Configuration

| Setting | Docker Setup | Windows Setup |
|---------|--------------|---------------|
| IP Address | `host.docker.internal` | `127.0.0.1` |
| Port | `102` | `102` |
| PLC Type | S7-1200 or S7-1500 | S7-1200 or S7-1500 |
| Rack | `0` | `0` |
| Slot | `1` | `1` |

**Test Variables:**
- `DB1,INT0` ? `42`
- `DB1,BYTE1` ? `100`
- `DB2,BYTE0` ? `1`

## Troubleshooting Index

### Permission Denied on Port 102
? See `FIX_PERMISSION_DENIED.md`

### Port 102 Already in Use
? Run `.\check_port_102.ps1`

### Node-RED Can't Connect (Docker)
? Check you're using `host.docker.internal`, not `localhost`

### Build Errors
? See `BUILD_STATUS.md`

### Missing snap7.dll
? Run `.\setup_snap7.ps1`

### Docker Not Working
? See `DOCKER_SETUP.md` troubleshooting section

## Quick Start (Docker)

```powershell
# 1. Setup Snap7 (first time only)
.\setup_snap7.ps1

# 2. Build in Visual Studio
# Open S7Server.sln, build as Debug|x64

# 3. Start everything
.\start-environment.ps1

# 4. Open Node-RED
# Browser opens automatically to http://localhost:1880

# 5. Configure S7 node
# IP: host.docker.internal
# Port: 102

# 6. Test connection
# Add S7 in node, read DB1,INT0 ? should get 42
```

## Quick Start (Windows Only)

```powershell
# 1. Setup Snap7 (first time only)
.\setup_snap7.ps1

# 2. Build in Visual Studio
# Open S7Server.sln, build as Release|x64

# 3. Install Node-RED (first time only)
npm install -g node-red node-red-contrib-s7

# 4. Start S7 Server (as Administrator)
cd x64\Release
.\S7Server.exe

# 5. Start Node-RED (new terminal)
node-red

# 6. Configure and test (same as Docker)
```

## Project Structure

```
S7-Server-Iso-on-tcp/
?
??? S7Server/      # Main project
?   ??? main.cpp           # Server implementation
?   ??? snap7/             # Snap7 library files
?   ??? S7Server.vcxproj   # Visual Studio project
?
??? x64/    # Build output
?   ??? Debug/    # Debug build
?   ??? Release/ # Release build
?
??? Documentation/    # Guides (*.md files)
?
??? Scripts/               # Helper scripts
?   ??? setup_snap7.ps1    # Snap7 setup
?   ??? check_port_102.ps1 # Port checker
?   ??? start-environment.ps1  # Docker starter
?   ??? stop-environment.ps1   # Docker stopper
?
??? Docker/       # Docker configuration
    ??? docker-compose.yml # Docker setup
    ??? .dockerignore   # Docker ignore rules
```

## Next Steps

1. Choose your setup option (Docker recommended)
2. Follow the appropriate quick start guide
3. Configure Node-RED with correct connection settings
4. Create your test flows
5. Read detailed docs if you encounter issues

## Need Help?

Check these files in order:

1. **Quick problems** ? `DOCKER_QUICKREF.md` or `QUICKSTART.md`
2. **Port issues** ? `FIX_PERMISSION_DENIED.md`
3. **Docker issues** ? `DOCKER_SETUP.md`
4. **Build issues** ? `BUILD_STATUS.md`
5. **Everything else** ? `README.md`

---

**Happy Testing! ??**

Choose your setup and get started in minutes!
