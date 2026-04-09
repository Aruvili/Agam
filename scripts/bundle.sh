#!/bin/bash
set -e

# Agam Shell Bundle Script
# Usage: ./scripts/bundle.sh <version> <platform>

VERSION=$1
PLATFORM=$2

if [ -z "$VERSION" ] || [ -z "$PLATFORM" ]; then
    echo "Usage: ./scripts/bundle.sh <version> <platform>"
    echo "Example: ./scripts/bundle.sh 1.0.0 windows-x64"
    exit 1
fi

BUNDLE_DIR="agam-${VERSION}-${PLATFORM}"
mkdir -p "${BUNDLE_DIR}/bin"

echo "Packaging Agam ${VERSION} for ${PLATFORM}..."

# 1. Copy Compiler
if [ -f "build/bin/agamc" ]; then
    cp build/bin/agamc "${BUNDLE_DIR}/bin/"
elif [ -f "build/bin/agamc.exe" ]; then
    cp build/bin/agamc.exe "${BUNDLE_DIR}/bin/"
else
    echo "Error: agamc not found. Build it first!"
    exit 1
fi

# 2. Copy Linker (Assuming lld is built/present)
if [ -f "build/bin/lld" ]; then
    cp build/bin/lld "${BUNDLE_DIR}/bin/agam-ld"
elif [ -f "build/bin/lld.exe" ]; then
    cp build/bin/lld.exe "${BUNDLE_DIR}/bin/agam-ld.exe"
fi

# 3. Copy Standard Library
if [ -d "std" ]; then
    cp -r std "${BUNDLE_DIR}/"
else
    echo "Warning: std library not found!"
fi

# 4. Create Archive
if [[ "$PLATFORM" == *"windows"* ]]; then
    # Create zip on windows if possible, otherwise tar.gz
    if command -v zip >/dev/null; then
        zip -r "${BUNDLE_DIR}.zip" "${BUNDLE_DIR}"
    else
        tar -czf "${BUNDLE_DIR}.zip" "${BUNDLE_DIR}"
    fi
    echo "Created ${BUNDLE_DIR}.zip"
else
    tar -czf "${BUNDLE_DIR}.tar.gz" "${BUNDLE_DIR}"
    echo "Created ${BUNDLE_DIR}.tar.gz"
fi

# Cleanup
rm -rf "${BUNDLE_DIR}"
echo "Bundle complete."
