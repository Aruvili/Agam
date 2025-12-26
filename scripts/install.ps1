# அகம் (agam) - Windows PowerShell Installer
# Run: irm https://agam.org/install.ps1 | iex
# Or:  .\install.ps1

param(
    [string]$InstallDir = "$env:LOCALAPPDATA\agam",
    [switch]$AddToPath = $true
)

$ErrorActionPreference = "Stop"
$Version = "0.1.3"

# ASCII Art Header
Write-Host ""
Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║     அகம் (agam) Installer v$Version                      ║" -ForegroundColor Cyan
Write-Host "║     தமிழ் நிரலாக்க மொழி - Tamil Programming Language         ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Detect architecture
$Arch = if ([Environment]::Is64BitOperatingSystem) { "x86_64" } else { "x86" }
Write-Host "Detected: Windows ($Arch)" -ForegroundColor Green

# Create install directory
Write-Host "Creating install directory: $InstallDir" -ForegroundColor Blue
New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
New-Item -ItemType Directory -Path "$InstallDir\examples" -Force | Out-Null
New-Item -ItemType Directory -Path "$InstallDir\docs" -Force | Out-Null

# Download URL
$DownloadUrl = "https://github.com/agam/agam/releases/latest/download/agam-windows-x86_64.zip"
$ZipPath = "$env:TEMP\agam.zip"
$ExtractPath = "$env:TEMP\agam-extract"

# Check for local binary first
$LocalBinary = ".\agam.exe"
$ReleaseBinary = ".\target\release\agam.exe"

if (Test-Path $LocalBinary) {
    Write-Host "Using local binary..." -ForegroundColor Green
    Copy-Item $LocalBinary -Destination "$InstallDir\agam.exe" -Force
} elseif (Test-Path $ReleaseBinary) {
    Write-Host "Using release binary..." -ForegroundColor Green
    Copy-Item $ReleaseBinary -Destination "$InstallDir\agam.exe" -Force
} else {
    Write-Host "Downloading agam..." -ForegroundColor Blue
    try {
        Invoke-WebRequest -Uri $DownloadUrl -OutFile $ZipPath -UseBasicParsing
        
        # Extract
        Write-Host "Extracting..." -ForegroundColor Blue
        if (Test-Path $ExtractPath) { Remove-Item $ExtractPath -Recurse -Force }
        Expand-Archive -Path $ZipPath -DestinationPath $ExtractPath -Force
        
        # Copy files
        Copy-Item "$ExtractPath\agam.exe" -Destination "$InstallDir\" -Force
        if (Test-Path "$ExtractPath\examples") {
            Copy-Item "$ExtractPath\examples\*" -Destination "$InstallDir\examples\" -Recurse -Force
        }
        if (Test-Path "$ExtractPath\docs") {
            Copy-Item "$ExtractPath\docs\*" -Destination "$InstallDir\docs\" -Recurse -Force
        }
        
        # Cleanup
        Remove-Item $ZipPath -Force -ErrorAction SilentlyContinue
        Remove-Item $ExtractPath -Recurse -Force -ErrorAction SilentlyContinue
    } catch {
        Write-Host "Download failed. Please download manually from:" -ForegroundColor Red
        Write-Host "  https://github.com/agam/agam/releases" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Or build from source: cargo build --release" -ForegroundColor Yellow
        exit 1
    }
}

# Copy local examples if available
if (Test-Path ".\examples") {
    Copy-Item ".\examples\*" -Destination "$InstallDir\examples\" -Recurse -Force -ErrorAction SilentlyContinue
}
if (Test-Path ".\docs") {
    Copy-Item ".\docs\*" -Destination "$InstallDir\docs\" -Recurse -Force -ErrorAction SilentlyContinue
}

# Add to PATH
if ($AddToPath) {
    Write-Host "Adding to PATH..." -ForegroundColor Blue
    
    $CurrentPath = [Environment]::GetEnvironmentVariable("Path", "User")
    if ($CurrentPath -notlike "*$InstallDir*") {
        $NewPath = "$InstallDir;$CurrentPath"
        [Environment]::SetEnvironmentVariable("Path", $NewPath, "User")
        $env:Path = "$InstallDir;$env:Path"
        Write-Host "Added to user PATH" -ForegroundColor Green
    } else {
        Write-Host "Already in PATH" -ForegroundColor Yellow
    }
}

# Create desktop shortcut (optional)
$CreateShortcut = Read-Host "Create desktop shortcut? (y/N)"
if ($CreateShortcut -eq 'y' -or $CreateShortcut -eq 'Y') {
    $Desktop = [Environment]::GetFolderPath("Desktop")
    $WScriptShell = New-Object -ComObject WScript.Shell
    $Shortcut = $WScriptShell.CreateShortcut("$Desktop\agam.lnk")
    $Shortcut.TargetPath = "$InstallDir\agam.exe"
    $Shortcut.WorkingDirectory = $InstallDir
    $Shortcut.Description = "அகம் - Tamil Programming Language"
    $Shortcut.Save()
    Write-Host "Desktop shortcut created" -ForegroundColor Green
}

# Verify installation
Write-Host ""
if (Test-Path "$InstallDir\agam.exe") {
    Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Green
    Write-Host "║  ✅ நிறுவல் வெற்றி! (Installation Successful!)              ║" -ForegroundColor Green
    Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Green
    Write-Host ""
    Write-Host "Usage:" -ForegroundColor Cyan
    Write-Host "  agam                    # Start REPL"
    Write-Host "  agam program.agam    # Run a file"
    Write-Host "  agam --help             # Show help"
    Write-Host ""
    Write-Host "Installation directory: $InstallDir" -ForegroundColor Gray
    Write-Host "Examples: $InstallDir\examples\" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Try: " -NoNewline; Write-Host 'அச்சிடு("வணக்கம் உலகம்!")' -ForegroundColor Yellow
    Write-Host ""
    Write-Host "NOTE: Restart your terminal for PATH changes to take effect." -ForegroundColor Yellow
} else {
    Write-Host "Installation may have failed. Check $InstallDir" -ForegroundColor Red
}
