#!/bin/bash
# அகம் (agam) - Unix Installer Script
# Works on Linux and macOS

set -e

VERSION="0.1.3"
INSTALL_DIR="/usr/local/bin"
agam_HOME="$HOME/.agam"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}"
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     அகம் (agam) Installer v${VERSION}                      ║"
echo "║     தமிழ் நிரலாக்க மொழி - Tamil Programming Language         ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Detect OS
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Linux*)     PLATFORM="linux";;
    Darwin*)    PLATFORM="macos";;
    *)          echo -e "${RED}Unsupported OS: $OS${NC}"; exit 1;;
esac

case "$ARCH" in
    x86_64)     ARCH_NAME="x86_64";;
    aarch64|arm64)    ARCH_NAME="aarch64";;
    *)          echo -e "${RED}Unsupported architecture: $ARCH${NC}"; exit 1;;
esac

echo -e "${GREEN}Detected: $PLATFORM ($ARCH_NAME)${NC}"

# Check if running with sudo
if [ "$EUID" -ne 0 ] && [ "$INSTALL_DIR" = "/usr/local/bin" ]; then
    echo -e "${YELLOW}Note: Installing to /usr/local/bin requires sudo${NC}"
    echo -e "You can also install to ~/.local/bin by running:"
    echo -e "  INSTALL_DIR=~/.local/bin ./install.sh"
    echo ""
    SUDO="sudo"
else
    SUDO=""
fi

# Download URL (update with actual release URL)
DOWNLOAD_URL="https://github.com/agam/agam/releases/latest/download/agam-${PLATFORM}-${ARCH_NAME}.tar.gz"

# Create temp directory
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

echo -e "${BLUE}Downloading agam...${NC}"

# Check if we have the binary locally (for local installs)
if [ -f "./agam" ]; then
    echo -e "${GREEN}Using local binary...${NC}"
    cp ./agam "$TEMP_DIR/"
elif command -v curl &> /dev/null; then
    curl -L -o "$TEMP_DIR/agam.tar.gz" "$DOWNLOAD_URL" 2>/dev/null || {
        echo -e "${RED}Download failed. Using local binary if available.${NC}"
        if [ -f "./target/release/agam" ]; then
            cp ./target/release/agam "$TEMP_DIR/"
        else
            echo -e "${RED}No binary found. Build with: cargo build --release${NC}"
            exit 1
        fi
    }
    if [ -f "$TEMP_DIR/agam.tar.gz" ]; then
        tar -xzf "$TEMP_DIR/agam.tar.gz" -C "$TEMP_DIR"
    fi
elif command -v wget &> /dev/null; then
    wget -q -O "$TEMP_DIR/agam.tar.gz" "$DOWNLOAD_URL" || {
        echo -e "${RED}Download failed.${NC}"
        exit 1
    }
    tar -xzf "$TEMP_DIR/agam.tar.gz" -C "$TEMP_DIR"
else
    echo -e "${RED}curl or wget required for download${NC}"
    exit 1
fi

# Install binary
echo -e "${BLUE}Installing to $INSTALL_DIR...${NC}"
$SUDO mkdir -p "$INSTALL_DIR"
$SUDO cp "$TEMP_DIR/agam" "$INSTALL_DIR/"
$SUDO chmod +x "$INSTALL_DIR/agam"

# Create agam home directory
mkdir -p "$agam_HOME"
mkdir -p "$agam_HOME/examples"
mkdir -p "$agam_HOME/docs"

# Copy examples and docs if available
if [ -d "./examples" ]; then
    cp -r ./examples/* "$agam_HOME/examples/" 2>/dev/null || true
fi
if [ -d "./docs" ]; then
    cp -r ./docs/* "$agam_HOME/docs/" 2>/dev/null || true
fi

# Verify installation
if command -v agam &> /dev/null; then
    echo -e "${GREEN}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║  ✅ நிறுவல் வெற்றி! (Installation Successful!)              ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    echo ""
    echo "Usage:"
    echo "  agam                    # Start REPL"
    echo "  agam program.agam    # Run a file"
    echo "  agam --help             # Show help"
    echo ""
    echo "Examples: $agam_HOME/examples/"
    echo "Docs: $agam_HOME/docs/"
    echo ""
    echo -e "${BLUE}Try: அச்சிடு(\"வணக்கம் உலகம்!\")${NC}"
else
    echo -e "${YELLOW}Binary installed but not in PATH.${NC}"
    echo "Add to your shell profile:"
    echo "  export PATH=\"$INSTALL_DIR:\$PATH\""
fi
