#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"

if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
else
    JOBS=2
fi
JOBS="${JOBS:-2}"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
cmake --build "$BUILD_DIR" --parallel "$JOBS"

echo "Готово: $BUILD_DIR/vhappvectordim"
