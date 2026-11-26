#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/run_example.sh" \
    data/Qwen3.5-0.8B-bf16.token_embd.weight_nr0s.bin
