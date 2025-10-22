#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
QTQMAKE="/opt/homebrew/opt/qt@5/bin/qmake"
if ! command -v "$QTQMAKE" >/dev/null 2>&1; then
	echo "qmake not found at $QTQMAKE" >&2
	exit 1
fi
echo "Running qmake..."
$QTQMAKE -o Makefile numerical_methods.pro

echo "Patching generated Makefile to remove AGL references..."
# macOS sed requires '' for in-place
sed -i '' 's/ -framework AGL//g' Makefile || true
sed -i '' '/AGL.framework/d' Makefile || true

echo "Building..."
make -j"$(sysctl -n hw.ncpu)"

echo "Build finished." 
