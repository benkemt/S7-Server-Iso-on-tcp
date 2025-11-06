# Snap7 Setup Script for Windows
# This script sets up the Snap7 library for the S7Server project from a local clone

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Snap7 Library Setup Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Define paths
$ProjectRoot = $PSScriptRoot
$Snap7Dir = Join-Path $ProjectRoot "S7Server\snap7"
$Snap7SourceDir = "D:\Source\Repos\snap7"

Write-Host "Project Root: $ProjectRoot" -ForegroundColor Gray
Write-Host "Snap7 Directory: $Snap7Dir" -ForegroundColor Gray
Write-Host "Snap7 Source: $Snap7SourceDir" -ForegroundColor Gray
Write-Host ""

# Check if Snap7 source directory exists
if (-not (Test-Path $Snap7SourceDir)) {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "ERROR: Snap7 source directory not found!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "Expected path: $Snap7SourceDir" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Please ensure the Snap7 repository is cloned at the expected location." -ForegroundColor White
    Write-Host "If it's at a different location, update the script's Snap7SourceDir variable." -ForegroundColor White
    Write-Host ""
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

# Create snap7 directory if it doesn't exist
if (-not (Test-Path $Snap7Dir)) {
    Write-Host "Creating snap7 directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Force -Path $Snap7Dir | Out-Null
}

# Check if files already exist
$RequiredFiles = @("snap7.h", "snap7.lib", "snap7.dll")
$FilesExist = $true
foreach ($file in $RequiredFiles) {
    if (-not (Test-Path (Join-Path $Snap7Dir $file))) {
        $FilesExist = $false
        break
    }
}

if ($FilesExist) {
    Write-Host "Snap7 files already exist in $Snap7Dir" -ForegroundColor Green
    Write-Host ""
    Write-Host "Existing files:" -ForegroundColor Green
    foreach ($file in $RequiredFiles) {
        $filePath = Join-Path $Snap7Dir $file
        if (Test-Path $filePath) {
        $fileSize = (Get-Item $filePath).Length
   Write-Host "  - $file ($fileSize bytes)" -ForegroundColor Gray
        }
    }
    Write-Host ""
    $response = Read-Host "Do you want to re-copy and overwrite? (y/N)"
    if ($response -ne "y" -and $response -ne "Y") {
    Write-Host "Setup cancelled. Existing files will be used." -ForegroundColor Yellow
        exit 0
    }
}

# Define source paths for the cloned repository
$HeaderSource = Join-Path $Snap7SourceDir "release\wrappers\c-cpp\snap7.h"
$LibSource = Join-Path $Snap7SourceDir "build\bin\win64\snap7.lib"
$DllSource = Join-Path $Snap7SourceDir "build\bin\win64\snap7.dll"

Write-Host "Checking for required files..." -ForegroundColor Yellow
Write-Host ""

# Check if build files exist
$NeedToBuild = $false
if (-not (Test-Path $LibSource) -or -not (Test-Path $DllSource)) {
  Write-Host "Build files not found. You need to build Snap7 first." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Please follow these steps:" -ForegroundColor White
    Write-Host "1. Open Visual Studio 2022" -ForegroundColor Gray
    Write-Host "2. Open the solution: $Snap7SourceDir\build\windows\VS2022\snap7.sln" -ForegroundColor Gray
    Write-Host "3. Select 'Release' and 'x64' configuration" -ForegroundColor Gray
    Write-Host "4. Build the solution" -ForegroundColor Gray
    Write-Host "5. Run this script again" -ForegroundColor Gray
    Write-Host ""
    $NeedToBuild = $true
}

if (-not (Test-Path $HeaderSource)) {
    Write-Host "  - snap7.h NOT FOUND at $HeaderSource" -ForegroundColor Red
    $NeedToBuild = $true
}

if ($NeedToBuild) {
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}

# Copy files
$success = $true

Write-Host "Copying files..." -ForegroundColor Yellow
Write-Host ""

if (Test-Path $HeaderSource) {
    Copy-Item $HeaderSource -Destination $Snap7Dir -Force
 Write-Host "  ? snap7.h copied" -ForegroundColor Green
} else {
    Write-Host "  ? snap7.h NOT FOUND at $HeaderSource" -ForegroundColor Red
 $success = $false
}

if (Test-Path $LibSource) {
    Copy-Item $LibSource -Destination $Snap7Dir -Force
  Write-Host "  ? snap7.lib copied" -ForegroundColor Green
} else {
 Write-Host "  ? snap7.lib NOT FOUND at $LibSource" -ForegroundColor Red
    $success = $false
}

if (Test-Path $DllSource) {
    Copy-Item $DllSource -Destination $Snap7Dir -Force
    Write-Host "  ? snap7.dll copied" -ForegroundColor Green
} else {
    Write-Host "  ? snap7.dll NOT FOUND at $DllSource" -ForegroundColor Red
    $success = $false
}

Write-Host ""

if ($success) {
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Setup completed successfully!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Files copied to: $Snap7Dir" -ForegroundColor White
    Write-Host ""
    foreach ($file in $RequiredFiles) {
        $filePath = Join-Path $Snap7Dir $file
        if (Test-Path $filePath) {
       $fileSize = (Get-Item $filePath).Length
        Write-Host "  - $file ($fileSize bytes)" -ForegroundColor Gray
        }
    }
    Write-Host ""
 Write-Host "You can now build the S7-Server-Iso-on-tcp project in Visual Studio 2022." -ForegroundColor White
    Write-Host ""
} else {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "Setup incomplete!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "Some files could not be found. Please ensure:" -ForegroundColor Yellow
    Write-Host "1. The Snap7 repository is cloned at: $Snap7SourceDir" -ForegroundColor Gray
 Write-Host "2. You have built the Snap7 project (VS2022, Release, x64)" -ForegroundColor Gray
    Write-Host ""
}

Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
