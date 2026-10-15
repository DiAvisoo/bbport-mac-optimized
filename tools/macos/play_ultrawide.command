#!/usr/bin/env bash
# Run the installed release at 3840x1080 using this checkout's fixed patch/startup pipeline.
# Settings and logs have a separate profile; saves use the installed app's normal folder.
# BB_GAME_DIR, BB_INSTALLED_APP, BB_DATA_DIR and BB_USER_DIR can override their locations.
set -euo pipefail
cd -- "$(dirname -- "$0")/../.."
engine=${BB_INSTALLED_APP:-/Applications/bbport.app}/Contents/Resources/bbport
[[ -x $engine/bin/bb-probe ]] || { echo "STOP: bbport.app is not installed at ${BB_INSTALLED_APP:-/Applications/bbport.app}" >&2; exit 1; }
game=${BB_GAME_DIR:-$(defaults read io.github.bbport.mac gameFolder 2>/dev/null || true)}
[[ -f $game/eboot.bin ]] || { echo "STOP: choose the game folder in bbport first, or set BB_GAME_DIR" >&2; exit 1; }
data=${BB_DATA_DIR:-$PWD/out/ultrawide}
mkdir -p "$data/out" "$data/patches" "$data/mods"
export BB_DATA_DIR=$data BB_GAME_DIR=$game BB_CONFIG=$data/bbport.ini
export BB_USER_DIR=${BB_USER_DIR:-$HOME/Library/Application Support/bbport/user}
export BB_PREBUILT=1 BB_PROBE=${BB_PROBE:-$engine/bin/bb-probe}
export BB_VULKAN_LIBRARY=${BB_VULKAN_LIBRARY:-$engine/lib/libvulkan.1.dylib}
export SDL_VULKAN_LIBRARY=$BB_VULKAN_LIBRARY
export VK_DRIVER_FILES=${VK_DRIVER_FILES:-$engine/lib/kosmickrisp/kosmickrisp_mesa_icd.json}
export MESA_SHADER_CACHE_DIR=${MESA_SHADER_CACHE_DIR:-$data/mesa-cache}
export PYTHONDONTWRITEBYTECODE=1
export BB_FPS=${BB_FPS:-60} BB_FULLSCREEN=${BB_FULLSCREEN:-1}
export BB_PRESENT_MODE=${BB_PRESENT_MODE:-Fifo}
provider=${BB_ULTRAWIDE_UPSCALER:-off}
preset=${BB_ULTRAWIDE_PRESET:-1}
case $provider in off|spatial|fsr3|metalfx) ;; *) echo "STOP: BB_ULTRAWIDE_UPSCALER must be off, spatial, fsr3 or metalfx" >&2; exit 1 ;; esac
case $preset in 0|1|2|3|4|5) ;; *) echo "STOP: BB_ULTRAWIDE_PRESET must be 0 through 5" >&2; exit 1 ;; esac
if [[ $provider != off && ${BB_ULTRAWIDE_UI_SUPPORT:-0} != 1 ]]; then
    echo "STOP: ultrawide upscaling needs the rebuilt renderer; use play_native_ultrawide.command" >&2
    exit 1
fi
[[ $provider != off ]] || preset=0
if [[ $provider == metalfx && $preset == 0 ]]; then
    echo "STOP: MetalFX needs an upscaling preset (1 through 5)" >&2
    exit 1
fi
runtime_provider=$provider
[[ $provider != spatial ]] || runtime_provider=off
export BB_UPSCALER=$runtime_provider BB_LIVE_RES=0 BB_UPSCALE_PRESET=$preset
export BB_PATCHES='Resolution Patch 3840x1080 (32:9)'
export BB_PATCHES_DIR=$data/patches BB_PATCHES_CONFIG=$data/patches.json
unset BB_RENDER_RES BB_OUTPUT_RES BB_UI_RES BB_AUTO_RENDER_RES
render_res=$(python3 - "$BB_CONFIG" "$provider" "$preset" <<'PY'
from pathlib import Path
import sys
sys.path.insert(0,'scripts')
from patches import scaled_sizes
path=Path(sys.argv[1])
provider,preset=sys.argv[2:]
if not path.exists():
    path.write_text('show_fps=1\nmodel_lod=0\neffect_dof=0\neffect_motion_blur=0\n'
                    'effect_dynamic_shadows=0\nskip_intro=1\n')
# Keep the container resolution at 1080p; the single community patch owns scene, UI and FOV.
keys={'output_res':'1920x1080','upscaler':'off' if provider=='spatial' else provider,
      'live_resolution':'0','preset':preset}
lines=[]
for line in path.read_text().splitlines():
    key,_,_=line.partition('=')
    if key.strip() not in keys: lines.append(line)
lines += [f'{key}={value}' for key,value in keys.items()]
path.write_text('\n'.join(lines)+'\n')
if provider!='off':
    scene,_=scaled_sizes({'output_res':'3840x1080','upscaler':'fsr3','preset':preset})
    print(f'{scene[0]}x{scene[1]}')
PY
)
if [[ $provider != off ]]; then
    export BB_RENDER_RES=$render_res BB_OUTPUT_RES=3840x1080 BB_UI_RES=3840x1080
    export BB_OBJECT_MOTION=${BB_OBJECT_MOTION:-1}
    # The generated resolution patch includes the 32:9 camera/HUD fixes itself.
    export BB_PATCHES=''
fi
log=$data/out/last-run.log
[[ ! -f $log ]] || cp "$log" "$data/out/previous-run.log"
exec > >(tee "$log") 2>&1
echo "Bloodborne: 3840x1080 (32:9), $provider ${render_res:-3840x1080}; settings and log in $data"
exec bash run.sh "$@"
