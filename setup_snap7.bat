@echo off
REM Snap7 Setup Script Launcher
REM This script launches the PowerShell setup script

echo ========================================
echo Snap7 Library Setup
echo ========================================
echo.
echo This script will help you set up the Snap7 library
echo for the S7Server project.
echo.
echo Press any key to continue...
pause >nul

powershell.exe -ExecutionPolicy Bypass -File "%~dp0setup_snap7.ps1"

echo.
pause
