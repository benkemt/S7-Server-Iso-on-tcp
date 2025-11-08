# Quick Reference: Docker + Windows Setup

## Problem
Node-RED in Docker container can't connect to S7 Server on Windows using `127.0.0.1` or `localhost`.

## Solution
Use the special hostname `host.docker.internal` to access Windows host services from Docker containers.

## Quick Start

### 1-Command Start (Recommended)
```powershell
# Run as Administrator
.\start-environment.ps1
```

This script will:
- ? Start S7 Server on Windows
- ? Start Node-RED in Docker
- ? Open Node-RED UI in browser
- ? Show connection details

### Manual Start

**Terminal 1 - S7 Server (as Administrator):**
```powershell
cd D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug
.\S7Server.exe
```

**Terminal 2 - Node-RED:**
```powershell
cd D:\Source\Repos\S7-Server-Iso-on-tcp
docker-compose up -d
```

### Stop Everything
```powershell
.\stop-environment.ps1
```

Or manually:
```powershell
docker-compose down
# Close S7 Server window or: Stop-Process -Name "S7Server" -Force
```

## Node-RED S7 Configuration

When adding an S7 node in Node-RED:

| Setting | Value |
|---------|-------|
| **IP Address** | `host.docker.internal` ?? |
| **Port** | `102` |
| **PLC Type** | S7-1200 or S7-1500 |
| **Rack** | `0` |
| **Slot** | `1` |

**Example Variables:**
- `DB1,INT0` ? `42`
- `DB1,BYTE1` ? `100`
- `DB2,BYTE0` ? `1`

## Troubleshooting

### Can't connect from Node-RED
1. Check S7 Server is running: `netstat -an | findstr :102`
2. Use `host.docker.internal` (NOT `127.0.0.1` or `localhost`)
3. Check Windows Firewall (see DOCKER_SETUP.md)

### S7 Server fails to start
- Run PowerShell as Administrator
- Check port 102 is not in use: `.\check_port_102.ps1`

### Node-RED container won't start
- Ensure Docker Desktop is running
- Check port 1880 is available
- Run: `docker-compose logs nodered`

## File Structure

```
S7-Server-Iso-on-tcp/
??? docker-compose.yml   # Docker Compose configuration
??? start-environment.ps1       # One-command startup script
??? stop-environment.ps1       # Shutdown script
??? DOCKER_SETUP.md      # Detailed documentation
??? x64/Debug/S7Server.exe      # S7 Server executable
```

## Architecture

```
??????????????????????????????????????
?      Windows Host         ?
?  ???????????????????????????????  ?
?  ? S7 Server :102?  ?
?  ???????????????????????????????  ?
?             ?      ?
?  ???????????????????????????????  ?
?  ? Docker:?  ?
?  ?  Node-RED :1880?  ?
?  ?  (host.docker.internal:102)  ?  ?
?  ???????????????????????????????  ?
??????????????????????????????????????
```

## Why This Works

- **S7 Server** runs natively on Windows ? full performance
- **Node-RED** runs in Docker ? isolated, reproducible
- **host.docker.internal** ? special DNS that Docker provides to access host

## See Also

- `DOCKER_SETUP.md` - Complete documentation
- `FIX_PERMISSION_DENIED.md` - Administrator privileges info
- `QUICKSTART.md` - General usage guide

---

**?? Tip:** Bookmark `http://localhost:1880` for quick access to Node-RED!
