#!/usr/bin/env bash
# Compatibility shortcut: the requested minimum is now a true 3840x1080 scene.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
export BB_ULTRAWIDE_UPSCALER=off BB_ULTRAWIDE_PRESET=0
exec bash tools/macos/play_native_ultrawide.command "$@"
