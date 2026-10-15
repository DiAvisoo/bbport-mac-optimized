#!/usr/bin/env bash
# Local-only self-contained release; no push, tag, upload or publication.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
[[ $(uname -s) == Darwin ]] || { echo 'Build this package on macOS.' >&2; exit 1; }
exec python3 tools/macos/package_optimized.py "${1:-0.1.0}"
