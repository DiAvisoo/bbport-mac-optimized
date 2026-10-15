# Bloodborne Optimized: user guide

## Requirements

- Apple Silicon and macOS 26 or later. Tested on M2 Max with 64 GB RAM; this preview uses M2 CPU tuning. Other models are untested.
- Rosetta 2 for the game's x86-64 code. If missing, install it through macOS or run `softwareupdate --install-rosetta --agree-to-license` in Terminal.
- Your own decrypted Bloodborne files, with update 1.09 merged into the base game. Choose the folder containing `eboot.bin`.

Python and the graphics libraries are bundled. Xcode, Homebrew and compilation are not required to play.

## Install and play

1. Extract `bbport-mac-optimized-0.1.0.zip`. Move **Bloodborne Optimized.app** into Applications. Keep an older working app under another name during testing.
2. Open it. This local preview is ad-hoc signed, not notarized. If macOS blocks it, attempt to open it once, then use **System Settings → Privacy & Security → Open Anyway**. Do not disable Gatekeeper.
3. Choose the game folder. Select resolution/fullscreen, then click **Play** (⌘R). **Stop** (⌘.) ends the game.

The first launch prepares game files and compiles shaders; it can take longer and stutter while new scenes are cached. No preparation files are written inside the app.

## Resolution and ultrawide

For full-resolution 32:9, enable **Ultrawide** and choose **3840×1080**. The launcher selects the matching camera/HUD patch and graphics memory size automatically, with upscaling disabled. Other available wide choices include 2560×720, 2560×1080 (21:9), and 3440×1440 (21:9).

Disable Ultrawide to return to standard 16:9. Change resolution while stopped, then start again. Do not enable an additional external resolution patch at the same time.

Reported performance is 30–40 FPS at 3840×1080 on M2 Max 64 GB. Higher resolutions cost more; 720p is available for speed at reduced image quality.

## Shortcuts and quicksave

The basic view shows the three configurable in-game shortcuts:

| Action | Default |
| --- | --- |
| Configuration menu | F1 |
| Quicksave | F2 |
| Quickload | F3 |

Click a shortcut, then press its replacement key. Conflicts are explained by name. F9 is reserved for developer input recording; Escape is reserved by the game/menu. Depending on keyboard settings, hold Fn/Globe when using function keys. The controller's L3+R3 configuration shortcut remains available.

Quicksave/quickload is off by default. Enable it in the basic view. Quicksave asks the game to write a fresh character/profile save, then snapshots the on-disk files once writes finish. Quickload restores the latest snapshot and restarts the game. It is **not a full memory save state**. Keep normal backups of valued progress. A failed or busy save does not replace the previous checkpoint.

## Advanced settings and saves

Open **Advanced** for save-folder selection, frame limit, effects, controller/keyboard settings, mods, external patches and diagnostics. Leave performance defaults unchanged for the tested profile. Live resolution and competing upscalers are disabled while using a wide patch.

- Default saves and shader data: `~/Library/Application Support/bbport/user`
- Launcher-generated files/settings/logs: `~/Library/Application Support/bbport/optimized-launcher`

Existing bbport saves are shared by default. For an isolated trial, copy your saves folder first and choose that copy under Advanced. The app never distributes or uploads your saves.

## Troubleshooting

- **Graphical glitches after loading:** restart the game; this often clears them. At 720p, an occasional startup failure has also occurred. This is a known issue, not a guaranteed fix.
- **Low FPS on the first run:** allow shader compilation to finish. Performance depends on the scene and display resolution.
- **Shortcut does nothing:** enable quicksave, check the legend/remap, and try Fn with function keys. Save only while in normal gameplay.
- **App runtime incomplete:** extract a fresh copy of the full ZIP; don't move individual files out of the app.

Report issues to [bbport-mac-optimized](https://github.com/DiAvisoo/bbport-mac-optimized/issues), including Mac model, macOS version, resolution and the relevant log excerpt. Avoid attaching game files or saves to public reports.

## Building and updating

Replace the app with a newer full package after testing; saves are outside it. For source builds, use [BUILD-OPTIMIZED.md](release/BUILD-OPTIMIZED.md). Matching source and license notices are supplied with the binary release.
