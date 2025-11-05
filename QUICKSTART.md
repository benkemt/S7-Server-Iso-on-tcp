# Quick Start Guide

Get your S7 Server up and running in 5 minutes!

## Step 1: Download Snap7

1. Visit [http://snap7.sourceforge.net/](http://snap7.sourceforge.net/)
2. Download the latest **Windows x64** version
3. Extract the archive to a folder (e.g., `C:\snap7`)

## Step 2: Run Setup Script

1. Double-click `setup_snap7.bat` in the project root
2. Follow the prompts
3. Select your extracted Snap7 folder when asked
4. Wait for the script to copy the necessary files

## Step 3: Build the Project

1. Open `S7Server.sln` in Visual Studio 2022
2. Select **Release|x64** from the toolbar
3. Press **F7** to build
4. Wait for the build to complete

## Step 4: Run the Server

1. Navigate to `x64\Release\`
2. Right-click `S7Server.exe`
3. Select **"Run as administrator"**
4. You should see "Server started successfully!"

## Step 5: Test with Node-RED

### Install Node-RED (if not already installed)

```bash
npm install -g node-red
node-red
```

### Install S7 Node

```bash
cd ~/.node-red
npm install node-red-contrib-s7
```

### Create a Test Flow

1. Open Node-RED in your browser (http://localhost:1880)
2. Drag an **S7 in** node to the canvas
3. Double-click to configure:
   - **PLC Type**: S7-1200
   - **IP Address**: 127.0.0.1
   - **Port**: 102
   - **Rack**: 0
   - **Slot**: 1
   - **Variable**: `DB1,INT0`
4. Connect to a **debug** node
5. Deploy the flow
6. Check the debug output - you should see value `42` (the test data)

## Troubleshooting

### "Failed to start server"
→ Run as Administrator (port 102 requires admin rights)

### "Cannot find snap7.dll"
→ Copy `S7Server\snap7\snap7.dll` to the same folder as `S7Server.exe`

### Node-RED can't connect
→ Check if the server shows "Client connected" message
→ Verify Windows Firewall isn't blocking port 102

## What's Next?

- Read the full [README.md](README.md) for detailed documentation
- Explore different memory areas (DB, I, Q, M, T, C)
- Modify the server code to add custom data
- Check [CONTRIBUTING.md](CONTRIBUTING.md) if you want to contribute

## Common Connection Settings

| Setting | Value |
|---------|-------|
| IP Address | 127.0.0.1 (local) or your PC's IP |
| Port | 102 |
| Rack | 0 |
| Slot | 1 |
| Connection Type | ISO-on-TCP |
| PLC Type | S7-1200 or S7-1500 |

## Example Node-RED Variables

```
DB1,INT0      - Read integer at byte 0 of DB1
DB1,BYTE1     - Read byte at byte 1 of DB1
DB1,REAL4     - Read 32-bit float at byte 4 of DB1
DB2,STRING0.10 - Read string of length 10 at byte 0 of DB2
I0.0          - Read input bit 0.0
Q0.0          - Read/write output bit 0.0
M0.0          - Read/write flag bit 0.0
```

## Support

For issues or questions, check:
- [README.md](README.md) - Full documentation
- [Snap7 Documentation](http://snap7.sourceforge.net/)
- [Node-RED S7 Node](https://flows.nodered.org/node/node-red-contrib-s7)

Happy Testing! 🎉
