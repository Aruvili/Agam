# Agam Installer Script for Windows (PowerShell)

Write-Host "Downloading the latest Agam release..."

$Asset = "agam-windows-x86_64.zip"
$Repo = "Aruvili/Agam"
$LatestUrl = "https://api.github.com/repos/$Repo/releases/latest"

try {
    $ReleaseInfo = Invoke-RestMethod -Uri $LatestUrl
    $AssetObj = $ReleaseInfo.assets | Where-Object { $_.name -eq $Asset }
    
    if (-not $AssetObj) {
        Write-Host "Error: Could not find the release for your platform." -ForegroundColor Red
        exit 1
    }

    $DownloadUrl = $AssetObj.browser_download_url
    $TempDir = Join-Path $env:TEMP "AgamInstall"

    if (Test-Path $TempDir) { Remove-Item -Recurse -Force $TempDir }
    New-Item -ItemType Directory -Force -Path $TempDir | Out-Null
    
    $ZipPath = Join-Path $TempDir $Asset

    Write-Host "Downloading $Asset..."
    Invoke-WebRequest -Uri $DownloadUrl -OutFile $ZipPath

    Write-Host "Extracting..."
    Expand-Archive -Path $ZipPath -DestinationPath $TempDir -Force

    $InstallDir = Join-Path $env:USERPROFILE ".agam"
    $BinDir = Join-Path $InstallDir "bin"
    $StdDir = Join-Path $InstallDir "std"

    if (Test-Path $InstallDir) { Remove-Item -Recurse -Force $InstallDir }
    New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
    New-Item -ItemType Directory -Force -Path $StdDir | Out-Null

    Move-Item -Path (Join-Path $TempDir "agamc.exe") -Destination $BinDir -Force
    Move-Item -Path (Join-Path $TempDir "std\*") -Destination $StdDir -Force

    # Add to PATH temporarily for this session
    $env:PATH += ";$BinDir"

    # Add to User PATH persistently
    $UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
    if ($UserPath -notlike "*$BinDir*") {
        $NewPath = $UserPath + ";$BinDir"
        [Environment]::SetEnvironmentVariable("PATH", $NewPath, "User")
    }

    # Set AGAM_STD_PATH
    [Environment]::SetEnvironmentVariable("AGAM_STD_PATH", $StdDir, "User")

    Write-Host "=========================================" -ForegroundColor Green
    Write-Host "Agam has been installed successfully! 🎉" -ForegroundColor Green
    Write-Host "Installed at: $InstallDir"
    Write-Host ""
    Write-Host "agamc is now in your PATH."
    Write-Host "Please restart your terminal to use the 'agamc' command."
    Write-Host "=========================================" -ForegroundColor Green

} catch {
    Write-Host "An error occurred during installation: $_" -ForegroundColor Red
    exit 1
}
