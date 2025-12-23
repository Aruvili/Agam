@echo off
REM அகம் (agam) - Windows Quick Install Batch Script
REM Double-click this file to install agam

echo.
echo ╔══════════════════════════════════════════════════════════════╗
echo ║     அகம் (agam) Quick Installer                        ║
echo ║     Tamil Programming Language                               ║
echo ╚══════════════════════════════════════════════════════════════╝
echo.

REM Check if PowerShell is available
where powershell >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo PowerShell is required for installation.
    echo Please install PowerShell or download manually from:
    echo   https://github.com/agam/agam/releases
    pause
    exit /b 1
)

echo Running PowerShell installer...
echo.

powershell -ExecutionPolicy Bypass -File "%~dp0install.ps1"

echo.
echo Press any key to close...
pause >nul
