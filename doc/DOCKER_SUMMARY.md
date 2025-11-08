# Docker Setup Complete! ??

## What Was Created

I've created a complete Docker setup that allows Node-RED to run in a container while connecting to your S7 Server running natively on Windows.

### Files Created

1. **`docker-compose.yml`** - Docker Compose configuration
   - Defines Node-RED service
   - Configures networking with `host.docker.internal`
   - Auto-installs `node-red-contrib-s7` node
   - Sets up data persistence

2. **`start-environment.ps1`** - Automated startup script
   - Checks prerequisites (Admin rights, Docker running)
- Starts S7 Server on Windows
   - Starts Node-RED container
   - Opens Node-RED UI in browser
   - Shows connection details

3. **`stop-environment.ps1`** - Shutdown script
   - Safely stops both services
   - Cleans up resources

4. **`DOCKER_SETUP.md`** - Complete documentation
   - Architecture explanation
   - Detailed troubleshooting
 - Firewall configuration
   - Network diagrams

5. **`DOCKER_QUICKREF.md`** - Quick reference guide
   - Common commands
   - Quick troubleshooting
   - Connection settings

6. **`.dockerignore`** - Docker optimization
   - Excludes unnecessary files from build context

## The Solution to Your Problem

### The Issue
When Node-RED runs in Docker, using `127.0.0.1` or `localhost` refers to the **container itself**, not your Windows host. That's why it couldn't connect.

### The Fix
Docker provides a special hostname: **`host.docker.internal`**

This hostname always resolves to the Docker host (your Windows machine), allowing containers to access services running on Windows.

## How to Use

### Super Easy Way (Recommended)

```powershell
# Run as Administrator
.\start-environment.ps1
```

That's it! The script will:
1. ? Start S7 Server on Windows (port 102)
2. ? Start Node-RED in Docker (port 1880)
3. ? Open http://localhost:1880 in your browser

### Configure Node-RED

In Node-RED, when adding an S7 node:

```
PLC Connection:
?? IP Address: host.docker.internal  ?? THIS IS THE KEY!
?? Port: 102
?? PLC Type: S7-1200
?? Rack: 0
?? Slot: 1

Test Variable: DB1,INT0  (should return 42)
```

### Stop Everything

```powershell
.\stop-environment.ps1
```

## Architecture Diagram

```
????????????????????????????????????????????????
?         Your Windows PC  ?
?      ?
?  ??????????????????????????????????????????  ?
?  ?     S7 Server (Native Windows)?  ?
?  ?     Listening on 0.0.0.0:102     ?  ?
?  ?  (All network interfaces) ?  ?
?  ??????????????????????????????????????????  ?
?   ?    ?
?   ?    ?
?  ??????????????????????????????????????????  ?
?  ?   Docker Bridge Network  ?  ?
?  ?   ?  ?
?  ?  ????????????????????????????????????  ?  ?
?  ?  ? Node-RED Container   ?  ?  ?
?  ?  ?  ?  ?  ?
?  ?  ? • Port 1880 (exposed to host)    ?  ?  ?
?  ?  ? • Connects to S7 via:?  ?  ?
?  ?  ?   host.docker.internal:102       ?  ?  ?
?  ?  ? • node-red-contrib-s7 installed  ?  ?  ?
?  ?  ? • Data persisted in volume       ?  ?  ?
?  ?  ????????????????????????????????????  ?  ?
?  ??????????????????????????????????????????  ?
?      ?
????????????????????????????????????????????????
  ?
        ?
   Your Browser
   http://localhost:1880
```

## Why This Approach?

### ? Advantages

1. **Best Performance** - S7 Server runs natively on Windows
2. **Easy Development** - Edit C++ code in Visual Studio, rebuild, restart
3. **Isolated Node-RED** - Container is reproducible, easy to reset
4. **No Port Conflicts** - Each service has its own port
5. **Persistent Data** - Node-RED flows saved in Docker volume
6. **Easy Setup** - One command to start everything

### ?? vs. Other Approaches

| Approach | S7 Server | Node-RED | Complexity | Performance |
|----------|-----------|----------|------------|-------------|
| **This Solution** | Windows Native | Docker | ?? Easy | ??? Best |
| Both on Windows | Windows Native | Windows Native | ? Easiest | ??? Best |
| Both in Docker | Docker/Linux | Docker | ??? Complex | ?? Good |
| Separate VMs | VM | VM | ???? Very Complex | ? Poor |

## Quick Commands

```powershell
# Start everything
.\start-environment.ps1

# Stop everything
.\stop-environment.ps1

# View Node-RED logs
docker-compose logs -f nodered

# Restart Node-RED only
docker-compose restart nodered

# Access Node-RED container shell
docker exec -it s7server-nodered sh

# Check if S7 Server is running
netstat -an | findstr :102

# Check Node-RED status
docker-compose ps
```

## Test Connection

Once both are running:

1. Open Node-RED: http://localhost:1880
2. Drag an **S7 in** node to the canvas
3. Double-click to configure:
 - PLC: Create new ? `host.docker.internal`, port 102
   - Variable: `DB1,INT0`
4. Connect to **debug** node
5. Click **Deploy**
6. Check debug panel ? should see `42`

## Troubleshooting Quick Fixes

### Node-RED can't connect
```
Connection Settings in Node-RED S7 Node:
? localhost:102
? 127.0.0.1:102
? host.docker.internal:102  ? USE THIS!
```

### S7 Server won't start
```powershell
# Check if port 102 is in use
.\check_port_102.ps1
```

### Docker not running
```
Start Docker Desktop, then retry
```

## Next Steps

1. ? Run `.\start-environment.ps1` as Administrator
2. ? Configure S7 node in Node-RED with `host.docker.internal:102`
3. ? Create your test flows
4. ? Read `DOCKER_SETUP.md` for advanced configuration

## Files for Reference

- **Quick Start**: `DOCKER_QUICKREF.md`
- **Full Docs**: `DOCKER_SETUP.md`
- **Port Issues**: `FIND_PORT_102_USER.md`
- **General Usage**: `QUICKSTART.md`

---

## Summary

? **Problem Solved**: Node-RED in Docker can now connect to S7 Server on Windows  
? **Solution**: Use `host.docker.internal` instead of `localhost`  
? **Easy to Use**: One command starts everything  
? **Professional Setup**: Docker Compose + automation scripts  
? **Well Documented**: Multiple guides for different needs  

**You're all set! ??**

Run `.\start-environment.ps1` and start building your Node-RED flows!
