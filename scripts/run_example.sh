#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BINARY="$BUILD_DIR/vhappvectordim"

if [[ $# -ne 1 ]]; then
    echo "Использование: $0 <файл с данными>" >&2
    echo "Пример: $0 data/test.bin" >&2
    exit 2
fi

FILE="$1"
if [[ "$FILE" != /* ]]; then
    FILE="$ROOT_DIR/$FILE"
fi

if [[ ! -f "$FILE" ]]; then
    echo "Ошибка: файл не найден: $FILE" >&2
    exit 1
fi

needs_build=0
if [[ ! -x "$BINARY" ]]; then
    needs_build=1
elif [[ -n "$(find "$ROOT_DIR/src" "$ROOT_DIR/CMakeLists.txt" \
        -newer "$BINARY" -print -quit 2>/dev/null)" ]]; then
    needs_build=1
fi

if (( needs_build )); then
    echo "Сборка проекта..." >&2
    if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
            -DCMAKE_BUILD_TYPE=Release
    fi

    if command -v nproc >/dev/null 2>&1; then
        JOBS="$(nproc)"
    else
        JOBS=2
    fi

    cmake --build "$BUILD_DIR" --parallel "${JOBS:-2}"
fi

exec "$BINARY" "$FILE"
