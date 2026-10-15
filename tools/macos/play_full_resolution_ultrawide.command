#!/usr/bin/env bash
# Native Apple Silicon renderer; actual scene and UI are both 3840x1080.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
export BB_ULTRAWIDE_UPSCALER=off BB_ULTRAWIDE_PRESET=0
exec bash tools/macos/play_native_ultrawide.command "$@"
