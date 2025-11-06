# Stop S7 Server + Node-RED Environment
# This script safely stops both services

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Stopping S7 Server + Node-RED" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$ProjectRoot = $PSScriptRoot

# Stop Node-RED container
Write-Host "Stopping Node-RED container..." -ForegroundColor Yellow
cd $ProjectRoot
docker-compose down

if ($LASTEXITCODE -eq 0) {
    Write-Host "? Node-RED stopped" -ForegroundColor Green
}
else {
    Write-Host "? Failed to stop Node-RED (might not be running)" -ForegroundColor Yellow
}

Write-Host ""

# Stop S7 Server
Write-Host "Stopping S7 Server..." -ForegroundColor Yellow
$s7Processes = Get-Process -Name "S7Server" -ErrorAction SilentlyContinue

if ($s7Processes) {
    foreach ($proc in $s7Processes) {
        Stop-Process -Id $proc.Id -Force
        Write-Host "? Stopped S7Server (PID: $($proc.Id))" -ForegroundColor Green
    }
}
else {
    Write-Host "? S7 Server not running" -ForegroundColor Yellow
}

Write-Host ""

# Check if port 102 is still in use
Start-Sleep -Seconds 2
$port102 = Get-NetTCPConnection -LocalPort 102 -ErrorAction SilentlyContinue
if ($port102) {
    Write-Host "WARNING: Port 102 is still in use!" -ForegroundColor Yellow
}
else {
    Write-Host "? Port 102 is free" -ForegroundColor Green
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Environment Stopped" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
