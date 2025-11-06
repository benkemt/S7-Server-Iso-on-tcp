# S7 Server + Node-RED Docker Environment Launcher
# This script starts both the S7 Server and Node-RED in the correct order

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "S7 Server + Node-RED Environment" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if running as Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

# Define paths
$ProjectRoot = $PSScriptRoot
$S7ServerExe = Join-Path $ProjectRoot "x64\Debug\S7Server.exe"

# Check if S7Server exists
if (-not (Test-Path $S7ServerExe)) {
    Write-Host "ERROR: S7Server.exe not found!" -ForegroundColor Red
    Write-Host "Expected location: $S7ServerExe" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Please build the project first in Visual Studio 2022." -ForegroundColor White
    Write-Host ""
    Write-Host "Press any key to exit..."
  $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

# Check if Docker is running
Write-Host "Checking Docker..." -ForegroundColor Yellow
try {
    $dockerVersion = docker version 2>&1
    if ($LASTEXITCODE -ne 0) {
throw "Docker not responding"
    }
    Write-Host "? Docker is running" -ForegroundColor Green
}
catch {
    Write-Host "ERROR: Docker is not running!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please start Docker Desktop and try again." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

# Check if port 102 is available
Write-Host "Checking port 102..." -ForegroundColor Yellow
$port102InUse = Get-NetTCPConnection -LocalPort 102 -ErrorAction SilentlyContinue
if ($port102InUse) {
    Write-Host "WARNING: Port 102 is already in use!" -ForegroundColor Yellow
    Write-Host ""
    $response = Read-Host "Do you want to kill the process using port 102? (y/N)"
    if ($response -eq "y" -or $response -eq "Y") {
    foreach ($conn in $port102InUse) {
       Stop-Process -Id $conn.OwningProcess -Force
            Write-Host "? Killed process $($conn.OwningProcess)" -ForegroundColor Green
        }
        Start-Sleep -Seconds 2
    }
    else {
        Write-Host "Cannot continue with port 102 in use." -ForegroundColor Red
   exit 1
    }
}

Write-Host "? Port 102 is available" -ForegroundColor Green
Write-Host ""

# Start S7 Server
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Starting S7 Server..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$s7ServerJob = Start-Process -FilePath $S7ServerExe -PassThru -WindowStyle Normal

# Wait for server to start
Write-Host "Waiting for S7 Server to start..." -ForegroundColor Yellow
Start-Sleep -Seconds 3

# Verify server is running
$port102Active = Get-NetTCPConnection -LocalPort 102 -ErrorAction SilentlyContinue
if ($port102Active) {
    Write-Host "? S7 Server started successfully (PID: $($s7ServerJob.Id))" -ForegroundColor Green
}
else {
    Write-Host "ERROR: S7 Server failed to start!" -ForegroundColor Red
    Write-Host "Check the S7 Server window for error messages." -ForegroundColor Yellow
    exit 1
}

Write-Host ""

# Start Node-RED container
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Starting Node-RED Container..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

cd $ProjectRoot

try {
    # Check if container already exists
    $existingContainer = docker ps -a --filter "name=s7server-nodered" --format "{{.Names}}" 2>$null
    
    if ($existingContainer) {
      Write-Host "Removing existing container..." -ForegroundColor Yellow
        docker-compose down 2>&1 | Out-Null
    }
    
    Write-Host "Starting Node-RED..." -ForegroundColor Yellow
    docker-compose up -d
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "? Node-RED container started" -ForegroundColor Green
    }
    else {
        throw "Failed to start Node-RED"
    }
}
catch {
    Write-Host "ERROR: Failed to start Node-RED!" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    Write-Host "Stopping S7 Server..." -ForegroundColor Yellow
    Stop-Process -Id $s7ServerJob.Id -Force
    exit 1
}

Write-Host ""
Write-Host "Waiting for Node-RED to initialize..." -ForegroundColor Yellow
Start-Sleep -Seconds 10

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Environment Started Successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Services Running:" -ForegroundColor White
Write-Host "  ? S7 Server:   Windows (PID: $($s7ServerJob.Id))" -ForegroundColor Gray
Write-Host "  ? Node-RED:    Docker Container" -ForegroundColor Gray
Write-Host ""
Write-Host "Access Points:" -ForegroundColor White
Write-Host "  • Node-RED UI:    http://localhost:1880" -ForegroundColor Cyan
Write-Host "  • S7 Server:      host.docker.internal:102" -ForegroundColor Cyan
Write-Host ""
Write-Host "Connection Settings for Node-RED S7 Node:" -ForegroundColor White
Write-Host "  - IP Address:    host.docker.internal" -ForegroundColor Yellow
Write-Host "  - Port:          102" -ForegroundColor Yellow
Write-Host "  - PLC Type:      S7-1200 or S7-1500" -ForegroundColor Yellow
Write-Host "  - Rack:          0" -ForegroundColor Yellow
Write-Host "  - Slot:    1" -ForegroundColor Yellow
Write-Host ""
Write-Host "Test Variables:" -ForegroundColor White
Write-Host "  - DB1,INT0    ? 42" -ForegroundColor Gray
Write-Host "  - DB1,BYTE1      ? 100" -ForegroundColor Gray
Write-Host "  - DB2,BYTE0      ? 1" -ForegroundColor Gray
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Opening Node-RED in browser..." -ForegroundColor Yellow
Start-Sleep -Seconds 2
Start-Process "http://localhost:1880"

Write-Host ""
Write-Host "To stop the environment, run: .\stop-environment.ps1" -ForegroundColor White
Write-Host "Or press Ctrl+C in this window and run: docker-compose down" -ForegroundColor White
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Environment is running. Monitoring logs..." -ForegroundColor Cyan
Write-Host "Press Ctrl+C to view cleanup instructions" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Monitor logs
try {
    docker-compose logs -f nodered
}
finally {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Yellow
    Write-Host "To stop the environment:" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "1. Stop S7 Server window (or run: Stop-Process -Id $($s7ServerJob.Id))" -ForegroundColor White
    Write-Host "2. Run: docker-compose down" -ForegroundColor White
    Write-Host ""
}
