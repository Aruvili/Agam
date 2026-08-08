#!/bin/bash
if [ -f "./build/bin/agamc" ]; then
    AGAMC="./build/bin/agamc"
elif [ -f "./build/bin/agamc.exe" ]; then
    AGAMC="./build/bin/agamc.exe"
elif [ -f "./build/bin/Release/agamc.exe" ]; then
    AGAMC="./build/bin/Release/agamc.exe"
else
    echo "Error: Agam compiler (agamc) executable not found in build directory."
    exit 1
fi

FILES=$(find . -name "*.agam" -not -path "./build/*" -not -path "./tests/samples/errors/*" -not -path "./std/*" -not -path "./packages/*/src/*" -not -path "./install_test/*" -not -path "./tests/diagnostics/*" -not -path "./tests/integration/math.agam" -not -name "*fail.agam")

echo "Checking Agam files..."
passCount=0
failCount=0
totalCount=0

for file in $FILES; do
    totalCount=$((totalCount + 1))
    echo "----------------------------------------"
    echo "Checking $file..."
    output=$($AGAMC "$file" 2>&1)
    status=$?
    if [ $status -eq 0 ]; then
        echo "PASS"
        passCount=$((passCount + 1))
    else
        echo "FAIL"
        failCount=$((failCount + 1))
        echo "$output"
    fi
done

echo "========================================"
echo "Total: $totalCount, PASS: $passCount, FAIL: $failCount"
if [ $failCount -ne 0 ]; then
    exit 1
fi
