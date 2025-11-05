# Snap7 Setup Script for Windows
# This script downloads and sets up the Snap7 library for the S7Server project

param(
    [string]$Snap7Version = "1.4.2"
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Snap7 Library Setup Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Define paths
$ProjectRoot = $PSScriptRoot
$Snap7Dir = Join-Path $ProjectRoot "S7Server\snap7"
$TempDir = Join-Path $ProjectRoot "temp_snap7"

Write-Host "Project Root: $ProjectRoot" -ForegroundColor Gray
Write-Host "Snap7 Directory: $Snap7Dir" -ForegroundColor Gray
Write-Host ""

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
    $response = Read-Host "Do you want to re-download and overwrite? (y/N)"
    if ($response -ne "y" -and $response -ne "Y") {
        Write-Host "Setup cancelled. Existing files will be used." -ForegroundColor Yellow
        exit 0
    }
}

# Information message
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "MANUAL DOWNLOAD REQUIRED" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""
Write-Host "Snap7 must be downloaded manually from:" -ForegroundColor White
Write-Host "http://snap7.sourceforge.net/" -ForegroundColor Cyan
Write-Host ""
Write-Host "Steps:" -ForegroundColor White
Write-Host "1. Visit the Snap7 website" -ForegroundColor Gray
Write-Host "2. Download the latest Windows release (x64)" -ForegroundColor Gray
Write-Host "3. Extract the archive to a temporary location" -ForegroundColor Gray
Write-Host ""
Write-Host "After downloading, you need to copy these files to:" -ForegroundColor White
Write-Host "  $Snap7Dir" -ForegroundColor Cyan
Write-Host ""
Write-Host "Required files:" -ForegroundColor White
Write-Host "  - snap7.h   (from snap7-full-x.x.x/include/)" -ForegroundColor Gray
Write-Host "  - snap7.lib (from snap7-full-x.x.x/build/bin/x64/)" -ForegroundColor Gray
Write-Host "  - snap7.dll (from snap7-full-x.x.x/build/bin/x64/)" -ForegroundColor Gray
Write-Host ""
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

$response = Read-Host "Have you already downloaded Snap7? (y/N)"
if ($response -eq "y" -or $response -eq "Y") {
    Write-Host ""
    Write-Host "Please select the Snap7 directory (the root folder of the extracted archive)..." -ForegroundColor Yellow
    
    # Prompt for Snap7 directory
    Add-Type -AssemblyName System.Windows.Forms
    $folderBrowser = New-Object System.Windows.Forms.FolderBrowserDialog
    $folderBrowser.Description = "Select the Snap7 root directory (e.g., snap7-full-1.4.2)"
    $folderBrowser.RootFolder = "MyComputer"
    
    if ($folderBrowser.ShowDialog() -eq "OK") {
        $Snap7SourceDir = $folderBrowser.SelectedPath
        Write-Host "Selected: $Snap7SourceDir" -ForegroundColor Gray
        Write-Host ""
        
        # Define source paths
        $HeaderSource = Join-Path $Snap7SourceDir "include\snap7.h"
        $LibSource = Join-Path $Snap7SourceDir "build\bin\x64\snap7.lib"
        $DllSource = Join-Path $Snap7SourceDir "build\bin\x64\snap7.dll"
        
        # Alternative paths (in case structure differs)
        if (-not (Test-Path $LibSource)) {
            $LibSource = Join-Path $Snap7SourceDir "release\Windows\x64\snap7.lib"
        }
        if (-not (Test-Path $DllSource)) {
            $DllSource = Join-Path $Snap7SourceDir "release\Windows\x64\snap7.dll"
        }
        
        # Check and copy files
        $success = $true
        
        Write-Host "Copying files..." -ForegroundColor Yellow
        
        if (Test-Path $HeaderSource) {
            Copy-Item $HeaderSource -Destination $Snap7Dir -Force
            Write-Host "  - snap7.h copied" -ForegroundColor Green
        } else {
            Write-Host "  - snap7.h NOT FOUND at $HeaderSource" -ForegroundColor Red
            $success = $false
        }
        
        if (Test-Path $LibSource) {
            Copy-Item $LibSource -Destination $Snap7Dir -Force
            Write-Host "  - snap7.lib copied" -ForegroundColor Green
        } else {
            Write-Host "  - snap7.lib NOT FOUND at $LibSource" -ForegroundColor Red
            $success = $false
        }
        
        if (Test-Path $DllSource) {
            Copy-Item $DllSource -Destination $Snap7Dir -Force
            Write-Host "  - snap7.dll copied" -ForegroundColor Green
        } else {
            Write-Host "  - snap7.dll NOT FOUND at $DllSource" -ForegroundColor Red
            $success = $false
        }
        
        Write-Host ""
        
        if ($success) {
            Write-Host "========================================" -ForegroundColor Green
            Write-Host "Setup completed successfully!" -ForegroundColor Green
            Write-Host "========================================" -ForegroundColor Green
            Write-Host ""
            Write-Host "You can now build the project in Visual Studio 2022." -ForegroundColor White
            Write-Host ""
        } else {
            Write-Host "========================================" -ForegroundColor Red
            Write-Host "Setup incomplete!" -ForegroundColor Red
            Write-Host "========================================" -ForegroundColor Red
            Write-Host ""
            Write-Host "Some files could not be found. Please check:" -ForegroundColor Yellow
            Write-Host "1. The Snap7 directory path is correct" -ForegroundColor Gray
            Write-Host "2. You downloaded the Windows x64 version" -ForegroundColor Gray
            Write-Host "3. The archive was fully extracted" -ForegroundColor Gray
            Write-Host ""
            Write-Host "You may need to manually copy the files to:" -ForegroundColor Yellow
            Write-Host "  $Snap7Dir" -ForegroundColor Cyan
            Write-Host ""
        }
    } else {
        Write-Host "Folder selection cancelled." -ForegroundColor Yellow
    }
} else {
    Write-Host "Please download Snap7 from http://snap7.sourceforge.net/" -ForegroundColor Yellow
    Write-Host "Then run this script again." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
