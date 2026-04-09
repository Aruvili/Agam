#!/bin/bash
AGAMC="./build/bin/agamc.exe"
FILES=$(find . -name "*.agam" -not -path "./build/*" -not -path "./tests/samples/errors/*" -not -path "./std/*" -not -path "./install_test/*" -not -path "./tests/diagnostics/*" -not -name "*fail.agam")

echo "Checking Agam files..."
for file in $FILES; do
    echo "----------------------------------------"
    echo "Checking $file..."
    $AGAMC "$file" 2>&1
    if [ $? -eq 0 ]; then
        echo "PASS"
    else
        echo "FAIL"
    fi
done
