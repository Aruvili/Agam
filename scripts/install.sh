#!/usr/bin/env bash
set -e

# Agam Interactive CLI Installer (Premium Edition)
# This script downloads and installs the Agam Toolchain.

# Colors & Styles
BOLD='\033[1m'
GREEN='\033[32m'
CYAN='\033[36m'
YELLOW='\033[33m'
RED='\033[31m'
MAGENTA='\033[35m'
RESET='\033[0m'

# Progress bar function
show_progress() {
    local duration=$1
    local width=40
    local progress=0
    while [ $progress -le 100 ]; do
        local filled=$((progress * width / 100))
        local empty=$((width - filled))
        printf "\r${CYAN}["
        printf "%${filled}s" | tr ' ' '#'
        printf "%${empty}s" | tr ' ' '-'
        printf "] %d%%${RESET}" $progress
        sleep 0.05
        progress=$((progress + 5))
    done
    echo ""
}

# 1. Visual Header
clear
echo -e "${MAGENTA}${BOLD}"
echo "     ___            ___           ___           ___     "
echo "    /  /\          /  /\         /  /\         /__/\    "
echo "   /  /::\        /  /::\       /  /::\        \  \:\   "
echo "  /  /:/\:\      /  /:/\:\     /  /:/\:\        \  \:\  "
echo " /  /:/~/::\    /  /:/  \:\   /  /:/~/::\   _____\__\:\ "
echo "/__/:/ /:/\:\  /__/:/ \__\:\ /__/:/ /:/\:\ /__/::::::::\\"
echo "\  \:\/:/__\/  \  \:\ /  /:/ \  \:\/:/__\/ \  \:\~~\~~\/"
echo " \  \::/        \  \:\  /:/   \  \::/       \  \:\  ~~~ "
echo "  \  \:\         \  \:\/:/     \  \:\        \  \:\     "
echo "   \  \:\         \  \::/       \  \:\        \  \:\    "
echo "    \__\/          \__\/         \__\/         \__\/    "
echo -e "         THE AGAM MEMORY-SAFE TOOLCHAIN${RESET}\n"

echo -e "${BOLD}Welcome to the Agam Programming Language installer!${RESET}\n"

# 2. Detect Platform
OS="$(uname -s)"
ARCH="$(uname -m)"

if [[ "$OS" == "Linux" && "$ARCH" == "x86_64" ]]; then
    ASSET="agam-linux-x86_64.tar.gz"
elif [[ "$OS" == "Darwin" && "$ARCH" == "arm64" ]]; then
    ASSET="agam-macos-aarch64.tar.gz"
else
    echo "Unsupported OS/Architecture: $OS / $ARCH"
    echo "Please compile from source or check the GitHub releases page."
    exit 1
fi

REPO="Aruvili/Agam" 
LATEST_URL="https://api.github.com/repos/$REPO/releases/latest"

# Get the download URL for the asset
DOWNLOAD_URL=$(curl -s $LATEST_URL | grep "browser_download_url.*$ASSET" | cut -d '"' -f 4)

if [[ -z "$DOWNLOAD_URL" ]]; then
    echo "Error: Could not find the release for your platform."
    exit 1
fi

TMP_DIR=$(mktemp -d)
cd $TMP_DIR

echo "Downloading $ASSET..."
curl -sL $DOWNLOAD_URL -o agam.tar.gz

echo "Extracting..."
tar -xzf agam.tar.gz

INSTALL_DIR="$HOME/.agam"
BIN_DIR="$INSTALL_DIR/bin"
STD_DIR="$INSTALL_DIR/std"

rm -rf "$INSTALL_DIR"
mkdir -p "$BIN_DIR"
mkdir -p "$STD_DIR"

mv agamc "$BIN_DIR/"
mv std/* "$STD_DIR/"

chmod +x "$BIN_DIR/agamc"

echo "========================================="
echo "Agam has been installed successfully! 🎉"
echo "Installed at: $INSTALL_DIR"
echo ""
echo "Please add the following to your ~/.bashrc or ~/.zshrc:"
echo "  export PATH=\"$BIN_DIR:\$PATH\""
echo "  export AGAM_STD_PATH=\"$STD_DIR\""
echo "========================================="
