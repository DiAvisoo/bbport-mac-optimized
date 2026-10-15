#!/usr/bin/env bash
# Same 3840x1080 profile as play_ultrawide.command, with the native GPU process.
# BB_NATIVE_GPU=0 compares the in-process renderer from the same source build.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
native=${BB_NATIVE_CHECKOUT:-$PWD/out/native-checkout}
[[ -x $native/out/bb-probe && -x $native/out/gpu-arm64/bb-gpu ]] || {
    echo "STOP: the native build is missing from $native" >&2
    exit 1
}
native=$(cd "$native" && pwd -P)
export BB_NATIVE_GPU=${BB_NATIVE_GPU:-1}
export BB_ULTRAWIDE_UPSCALER=${BB_ULTRAWIDE_UPSCALER:-off}
export BB_ULTRAWIDE_PRESET=${BB_ULTRAWIDE_PRESET:-0}
export BB_METALFX_EVENTS=${BB_METALFX_EVENTS:-1}
export BB_FPS=${BB_FPS:-uncap}
export BB_FRAMES_AHEAD=${BB_FRAMES_AHEAD:-1}
export BB_PROBE=$native/out/bb-probe
export BB_GPU_PROCESS=$native/out/gpu-arm64/bb-gpu
export BB_GPU_DEPS=${BB_GPU_DEPS:-$native/deps-arm64}
export BB_DATA_DIR=${BB_DATA_DIR:-$PWD/out/native-fullres}
mkdir -p "$BB_DATA_DIR/out"
# Start a full-resolution profile; existing effect preferences are retained.
if [[ ! -f $BB_DATA_DIR/bbport.ini ]]; then
    printf '%s\n' show_fps=1 model_lod=0 effect_dof=0 effect_motion_blur=0 \
        effect_dynamic_shadows=0 effect_game_aa=1 effect_chromatic_aberration=0 \
        effect_ssao=1 skip_intro=1 > "$BB_DATA_DIR/bbport.ini"
fi
prefix=$BB_GPU_DEPS
[[ $BB_NATIVE_GPU == 0 ]] && prefix=$native/deps-x86_64
renderer=$native/out/gpu-arm64/libbbgpu.dylib
[[ $BB_NATIVE_GPU == 0 ]] && renderer=$native/out/gpu/libbbgpu.dylib
if [[ $BB_ULTRAWIDE_UPSCALER != off ]] && ! /usr/bin/grep -aq BB_UI_RES "$renderer"; then
    echo "STOP: rebuild the renderer with ultrawide UI-coordinate support first" >&2
    exit 1
fi
export BB_ULTRAWIDE_UI_SUPPORT=1
export BB_VULKAN_LIBRARY=$prefix/lib/libvulkan.1.dylib
export VK_DRIVER_FILES=$prefix/lib/kosmickrisp/kosmickrisp_mesa_icd.json
export BB_FRAME_STATS=${BB_FRAME_STATS:-1}
export BB_FRAME_LOG=${BB_FRAME_LOG:-$BB_DATA_DIR/out/frames.csv}
# The renderer's existing diagnostic control can compare providers in one stationary scene.
if [[ -n ${BB_PRESET_FILE:-} ]]; then
    case $BB_ULTRAWIDE_UPSCALER in off|spatial) provider=0 ;; fsr3) provider=1 ;; metalfx) provider=5 ;; esac
    printf '%s 1 %s\n' "$BB_ULTRAWIDE_PRESET" "$provider" > "$BB_PRESET_FILE"
fi
echo "Renderer comparison: BB_NATIVE_GPU=$BB_NATIVE_GPU (1 = native Apple Silicon renderer)"
exec bash tools/macos/play_ultrawide.command "$@"
