# Check what's using port 102 and optionally kill it
# Run this as Administrator

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Port 102 Process Finder" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if running as Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "WARNING: Not running as Administrator!" -ForegroundColor Yellow
    Write-Host "Some information may not be available." -ForegroundColor Yellow
    Write-Host ""
}

# Find connections on port 102
Write-Host "Searching for processes using port 102..." -ForegroundColor Yellow
Write-Host ""

try {
    $connections = Get-NetTCPConnection -LocalPort 102 -ErrorAction Stop
    
    if ($connections) {
        Write-Host "Found $($connections.Count) connection(s) on port 102:" -ForegroundColor Yellow
        Write-Host ""
        
        $processesFound = @()
        
        foreach ($conn in $connections) {
            $process = Get-Process -Id $conn.OwningProcess -ErrorAction SilentlyContinue
            
            if ($process) {
                $processInfo = [PSCustomObject]@{
                    PID        = $conn.OwningProcess
                    ProcessName = $process.ProcessName
                    Path       = $process.Path
                    State      = $conn.State
                    LocalAddr  = $conn.LocalAddress
                }
    
                $processesFound += $processInfo
   
                Write-Host "??????????????????????????????????????" -ForegroundColor Gray
                Write-Host "? PID:          $($processInfo.PID)" -ForegroundColor White
                Write-Host "? Process Name: $($processInfo.ProcessName)" -ForegroundColor White
                Write-Host "? Path:      $($processInfo.Path)" -ForegroundColor Gray
                Write-Host "? State:        $($processInfo.State)" -ForegroundColor White
                Write-Host "? Local Addr:   $($processInfo.LocalAddr):102" -ForegroundColor White
                Write-Host "??????????????????????????????????????" -ForegroundColor Gray
                Write-Host ""
            }
        }
        
        # Ask if user wants to kill the processes
        if ($processesFound.Count -gt 0) {
            Write-Host "========================================" -ForegroundColor Yellow
            $response = Read-Host "Do you want to KILL these process(es)? (y/N)"
 
            if ($response -eq "y" -or $response -eq "Y") {
                Write-Host ""
                Write-Host "Stopping processes..." -ForegroundColor Yellow
    
                foreach ($proc in $processesFound) {
                    try {
                        Stop-Process -Id $proc.PID -Force
                        Write-Host "? Killed: $($proc.ProcessName) (PID: $($proc.PID))" -ForegroundColor Green
                    }
                    catch {
                        Write-Host "? Failed to kill: $($proc.ProcessName) (PID: $($proc.PID))" -ForegroundColor Red
                        Write-Host "  Error: $($_.Exception.Message)" -ForegroundColor Red
                    }
                }
   
                Write-Host ""
                Write-Host "Waiting 2 seconds..." -ForegroundColor Gray
                Start-Sleep -Seconds 2
  
                # Verify port is now free
                Write-Host ""
                Write-Host "Verifying port 102 is now free..." -ForegroundColor Yellow
                $stillInUse = Get-NetTCPConnection -LocalPort 102 -ErrorAction SilentlyContinue
    
                if ($stillInUse) {
                    Write-Host "WARNING: Port 102 is still in use!" -ForegroundColor Red
                    Write-Host "You may need to restart your computer." -ForegroundColor Yellow
                }
                else {
                    Write-Host "SUCCESS: Port 102 is now FREE!" -ForegroundColor Green
                    Write-Host "You can now start your S7Server." -ForegroundColor Green
                }
            }
            else {
                Write-Host ""
                Write-Host "No processes were killed." -ForegroundColor Yellow
                Write-Host ""
                Write-Host "Suggestions:" -ForegroundColor White
                Write-Host "  1. Close the application manually" -ForegroundColor Gray
                Write-Host "  2. Use Task Manager to end the process" -ForegroundColor Gray
                Write-Host "  3. Configure S7Server to use a different port (e.g., 10102)" -ForegroundColor Gray
                Write-Host "  4. Restart your computer" -ForegroundColor Gray
            }
        }
    }
    else {
        Write-Host "Port 102 is FREE - no processes using it!" -ForegroundColor Green
        Write-Host ""
        Write-Host "Your S7Server should be able to start successfully." -ForegroundColor Green
        Write-Host "Make sure to run S7Server as Administrator!" -ForegroundColor Yellow
    }
}
catch {
    Write-Host "Could not check port 102." -ForegroundColor Red
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host ""
    Write-Host "Trying alternative method with netstat..." -ForegroundColor Yellow
    Write-Host ""
    
    # Fallback to netstat
    $netstatOutput = netstat -ano | Select-String ":102"
    
    if ($netstatOutput) {
        Write-Host "Found port 102 in use:" -ForegroundColor Yellow
        Write-Host ""
        
        foreach ($line in $netstatOutput) {
            Write-Host $line -ForegroundColor White
            
            # Extract PID
            if ($line -match '\s+(\d+)\s*$') {
                $pid = $matches[1]
                $process = Get-Process -Id $pid -ErrorAction SilentlyContinue
               
                if ($process) {
                    Write-Host "  ? Process: $($process.ProcessName) (PID: $pid)" -ForegroundColor Gray
                    Write-Host "  ? Path: $($process.Path)" -ForegroundColor DarkGray
                }
            }
        }
    }
    else {
        Write-Host "Port 102 appears to be free!" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
