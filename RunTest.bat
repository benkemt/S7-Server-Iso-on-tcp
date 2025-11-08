@echo off
REM Quick test script for S7 Client/Server
REM This script starts both the server and client in separate windows

echo ========================================
echo S7 Client/Server Test Launcher
echo ========================================
echo.
echo This will start:
echo 1. S7 Server (requires Admin privileges for port 102)
echo 2. S7 Client (test application)
echo.
echo Press any key to continue...
pause > nul

REM Check if we have Release builds
if not exist "x64\Release\S7Server.exe" (
    echo ERROR: S7Server.exe not found in x64\Release\
    echo Please build the solution first.
    pause
    exit /b 1
)

if not exist "S7Client\x64\Release\S7Client.exe" (
    echo ERROR: S7Client.exe not found in S7Client\x64\Release\
    echo Please build the solution first.
    pause
    exit /b 1
)

echo.
echo Starting S7 Server in a new window...
echo (This requires Administrator privileges)
start "S7 Server" cmd /k "cd /d %~dp0x64\Release && S7Server.exe"

echo.
echo Waiting 3 seconds for server to start...
timeout /t 3 /nobreak > nul

echo.
echo Starting S7 Client in a new window...
start "S7 Client Test" cmd /k "cd /d %~dp0S7Client\x64\Release && S7Client.exe"

echo.
echo ========================================
echo Both applications started!
echo ========================================
echo.
echo Check the opened windows for results.
echo.
echo Press any key to exit this launcher...
pause > nul
