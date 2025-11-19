# Docker Setup for S7 Server + Node-RED

This setup allows Node-RED to run in a Docker container while connecting to the S7 Server running on your Windows host.

## Architecture

```
???????????????????????????????????????
?         Windows Host       ?
?       ?
?  ????????????????????????????????  ?
?  ?  S7 Server (Native Windows)  ?  ?
?  ?  Port: 102              ?  ?
?  ?  IP: host.docker.internal    ?  ?
?  ????????????????????????????????  ?
?               ?
?  ????????????????????????????????  ?
?  ?  Docker Container  ?  ?
?  ?  ??????????????????????????  ?  ?
?  ?  ?  Node-RED    ?  ?  ?
?  ?  ?  Port: 1880?  ?  ?
?  ?  ?  with S7 node          ?  ?  ?
?  ?  ??????????????????????????  ?  ?
?  ????????????????????????????????  ?
???????????????????????????????????????
```

## Prerequisites

- ? Docker Desktop for Windows installed
- ? S7 Server built and ready to run on Windows
- ? Port 102 available on Windows host
- ? Port 1880 available for Node-RED

## Quick Start

### 1. Start S7 Server on Windows

```powershell
# Run as Administrator
cd D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug
.\S7Server.exe
```

Wait for the message: `*** Server started successfully! ***`

### 2. Start Node-RED Container

In the project root directory:

```powershell
# Start Node-RED
docker-compose up -d

# View logs
docker-compose logs -f nodered
```

### 3. Access Node-RED

Open your browser:
```
http://localhost:1880
```

### 4. Configure S7 Connection in Node-RED

When configuring the S7 node in Node-RED, use these settings:

**Connection Settings:**
- **IP Address**: `host.docker.internal` ?? (NOT 127.0.0.1 or localhost)
- **Port**: `102`
- **PLC Type**: S7-1200 or S7-1500
- **Rack**: `0`
- **Slot**: `1`

**Example Variable:**
- `DB1,INT0` - Should read value `42`
- `DB1,BYTE1` - Should read value `100`
- `DB2,BYTE0` - Should read value `1`

## Important: Why host.docker.internal?

When Node-RED runs in a Docker container:
- ? `127.0.0.1` or `localhost` refers to the **container itself**, not Windows
- ? `192.168.x.x` might work but requires knowing your Windows IP
- ? `host.docker.internal` is a **special DNS name** that always points to the Docker host (Windows)

## Docker Commands

### Start Services
```powershell
docker-compose up -d
```

### Stop Services
```powershell
docker-compose down
```

### View Logs
```powershell
# All logs
docker-compose logs -f

# Just Node-RED
docker-compose logs -f nodered
```

### Restart Node-RED
```powershell
docker-compose restart nodered
```

### Rebuild Container (if you modify docker-compose.yml)
```powershell
docker-compose up -d --build
```

### Access Node-RED Shell
```powershell
docker exec -it s7server-nodered /bin/sh
```

## Firewall Configuration

If Node-RED can't connect, check Windows Firewall:

### Allow S7 Server Through Firewall

Run as Administrator:
```powershell
# Allow inbound on port 102
New-NetFirewallRule -DisplayName "S7 Server" -Direction Inbound -LocalPort 102 -Protocol TCP -Action Allow

# Or allow the S7Server.exe executable
New-NetFirewallRule -DisplayName "S7 Server App" -Direction Inbound -Program "D:\Source\Repos\S7-Server-Iso-on-tcp\x64\Debug\S7Server.exe" -Action Allow
```

## Troubleshooting

### Node-RED Can't Connect to S7 Server

1. **Verify S7 Server is running:**
   ```powershell
   netstat -an | findstr :102
   # Should show: TCP    0.0.0.0:102    0.0.0.0:0    LISTENING
   ```

2. **Test connection from container:**
   ```powershell
   docker exec -it s7server-nodered sh -c "ping -c 4 host.docker.internal"
   ```

3. **Check if Docker can reach port 102:**
   ```powershell
   docker exec -it s7server-nodered sh -c "nc -zv host.docker.internal 102"
   ```

4. **Check S7 Server logs** - Should show `[EVENT] Client connected` when Node-RED connects

5. **Verify Docker Desktop is using WSL 2 backend:**
   - Docker Desktop ? Settings ? General
   - Ensure "Use the WSL 2 based engine" is checked

### S7 Node Not Installed

If the S7 node isn't available in Node-RED:

```powershell
# Access container
docker exec -it s7server-nodered sh

# Install manually
cd /data
npm install node-red-contrib-s7

# Restart Node-RED
exit
docker-compose restart nodered
```

### Port 1880 Already in Use

```powershell
# Find what's using port 1880
netstat -ano | findstr :1880

# Change port in docker-compose.yml
# ports:
#   - "1881:1880"  # Use 1881 instead
```

## Data Persistence

Node-RED data (flows, credentials, nodes) is stored in a Docker volume:

```powershell
# View volumes
docker volume ls

# Backup Node-RED data
docker run --rm -v s7server_node_red_data:/data -v ${PWD}:/backup alpine tar czf /backup/nodered-backup.tar.gz -C /data .

# Restore Node-RED data
docker run --rm -v s7server_node_red_data:/data -v ${PWD}:/backup alpine tar xzf /backup/nodered-backup.tar.gz -C /data
```

## Example Node-RED Flow

After configuring the S7 connection, try this test flow:

1. Drag an **S7 in** node
2. Configure:
   - PLC: `host.docker.internal:102`
   - Variable: `DB1,INT0`
3. Connect to **debug** node
4. Deploy
5. Should see value `42` in debug panel

## Network Architecture Explained

```
Windows Host (192.168.1.100)
?
?? S7Server.exe listening on 0.0.0.0:102
?  ?? Accessible from:
?     - 127.0.0.1:102 (localhost)
?     - 192.168.1.100:102 (LAN IP)
?     - host.docker.internal:102 (from Docker)
?
?? Docker Bridge Network
   ?
   ?? Node-RED Container
      - Uses host.docker.internal to reach Windows
      - Exposes port 1880 to Windows (localhost:1880)
```

## Advanced: Running Both in Docker (Optional)

If you want to run **both** S7 Server and Node-RED in Docker (Linux containers), you'll need to build a Linux version of the S7 Server. This is more complex but possible.

See `DOCKER_FULL_SETUP.md` for instructions (to be created if needed).

## Summary

? S7 Server runs **natively on Windows** (best performance)  
? Node-RED runs in **Docker container** (isolated, easy to manage)  
? Containers use **host.docker.internal** to access Windows services  
? Data persists in **Docker volumes**  
? Easy to start/stop with **docker-compose**

---

**Need Help?** 
- Check S7 Server console for connection messages
- Check Node-RED debug panel for errors
- Review Docker logs: `docker-compose logs -f`
