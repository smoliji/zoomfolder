#!/bin/bash
set -e

SDK_PATH=$(xcrun --show-sdk-path 2>/dev/null)
if [ -z "$SDK_PATH" ]; then
    echo "Error: macOS SDK not found. Install Xcode Command Line Tools:"
    echo "  xcode-select --install"
    exit 1
fi

SDK_NAME=$(basename "$SDK_PATH")
OUT="sdk/${SDK_NAME}.tar.xz"

if [ -f "$OUT" ]; then
    echo "SDK already packaged: $OUT"
    exit 0
fi

echo "Packaging $SDK_PATH -> $OUT"
tar -cJf "$OUT" -C "$(dirname "$SDK_PATH")" "$SDK_NAME"
echo "Done: $OUT ($(du -h "$OUT" | cut -f1))"
