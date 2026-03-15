#!/usr/bin/env bash
set -euo pipefail

# Ensure script runs from its directory
cd "$(dirname "$0")"

# Path to qmake (Homebrew qt@5)
QTQMAKE="/opt/homebrew/opt/qt@5/bin/qmake"

if [ ! -x "$QTQMAKE" ]; then
    echo "Error: qmake not found at $QTQMAKE" >&2
    echo "Please ensure qt@5 is installed via Homebrew: brew install qt@5" >&2
    exit 1
fi

echo "Cleaning previous build..."
make clean >/dev/null 2>&1 || true
rm -f Makefile

echo "Running qmake..."
"$QTQMAKE" numerical_methods.pro

echo "Patching generated Makefile to remove deprecated AGL framework..."
# Remove -framework AGL from LIBS
sed -i '' 's/-framework AGL//g' Makefile
# Remove include path for AGL framework
sed -i '' 's/-I[^ ]*AGL[^ ]*//g' Makefile

echo "Building application..."
make -j"$(sysctl -n hw.ncpu)"

echo "Build successful! Application is located at:"
echo "$(pwd)/numerical_methods.app/Contents/MacOS/numerical_methods"
 
