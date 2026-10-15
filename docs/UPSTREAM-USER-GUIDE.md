> Historical upstream/development guide. For the packaged optimized app, see [USER_GUIDE.md](USER_GUIDE.md).

# bbport for Mac: user guide

How to install the app, set it up, play, and get help when something goes wrong. For what the
project is and its current state, see the [front page](../README.md).

- [What you need](#what-you-need)
- [Install](#install)
- [First launch](#first-launch)
- [The settings, section by section](#the-settings-section-by-section)
- [Recommended settings](#recommended-settings)
- [In the game](#in-the-game)
- [Where your files are](#where-your-files-are)
- [Updating](#updating)
- [Troubleshooting](#troubleshooting)
- [Reporting a problem](#reporting-a-problem)
- [Building from source](#building-from-source)

## What you need

- **A Mac with Apple Silicon** (M1 or later) and **macOS 26 or later**. So far the port has been
  tested on one machine, an M5 Max with macOS 27; reports from other Macs are very welcome.
- **Rosetta 2.** The game's own code is Intel (x86-64) code.
- **Python 3.** The app's start-up scripts use it. The Command Line Tools include it.
- **Your own copy of Bloodborne**, dumped from your own PS4: title ID **CUSA03173**, **merged
  with the 1.09 update** (the update's files copied over the base game's), and **decrypted**. The folder you choose is the one with `eboot.bin` in it. No game
  files come with the app, and none can be provided.

To install Rosetta 2 and the Command Line Tools, open **Terminal** (Applications ▸ Utilities)
and run these two commands. Each opens an installer or asks for your password; let it finish
before the next one. If they're already installed, they say so.

```
softwareupdate --install-rosetta --agree-to-license
```

```
xcode-select --install
```

## Install

1. Download the latest `bbport-mac-….zip` from the
   [Releases page](https://github.com/bmy/bbport-mac/releases).
2. Open the zip. You get a folder with **bbport.app**, a short read-me and the licence.
3. Drag **bbport.app** into your **Applications** folder. Opened straight from Downloads, macOS
   runs it from a read-only copy and the app will ask you to move it.
4. Open it. Because the app isn't notarised by Apple (it's a free hobby project without a paid
   developer account), macOS stops it the first time with *"Apple could not verify…"*. Click
   **Done**, then open **System Settings ▸ Privacy & Security**, scroll down to the message
   about bbport, click **Open Anyway** and confirm with your password or Touch ID.

   If macOS says instead that bbport *"is damaged and can't be opened"*, it isn't damaged: run
   this in Terminal, then open it again.

   ```
   xattr -dr com.apple.quarantine /Applications/bbport.app
   ```

## First launch

1. Under **Game**, click **Choose…** next to *Game folder* and pick the folder with `eboot.bin`.
   The line below it checks the folder: a green tick reads *Bloodborne CUSA03173, version
   01.09*. An orange warning (another title ID or version) still lets you play, but expect
   problems.
2. Leave the rest as it is for now, or see [Recommended settings](#recommended-settings).
3. Click **Play** (⌘R). The **Game Log** window opens and shows what the game prints, and the
   game's window appears.

**The first launch is slow.** The game compiles its graphics shaders the first time it needs
them, so the window can stay black for a few minutes and the game stutters whenever something
new appears. This is normal. Everything compiled is kept, and from the second launch on it
loads during a short black screen at startup.

**Stop** (⌘.) ends the game. Quitting the app also stops it.

## The settings, section by section

Settings are saved when the game starts. The upscaler, resolution, effects and similar game
settings live in `bbport.ini`, which the in-game menu also edits, so changes made in either
place show up in the other.

**Game**
- *Game folder*: the folder with `eboot.bin`.
- *Saves folder*: where saves and compiled shaders go. The default is fine; change it to keep
  saves somewhere else (for example to use saves from a source build, see
  [Where your files are](#where-your-files-are)).
- *System language*: the game's language.

**Display**
- *Output resolution*: the size of the final image. 2560×1440 is a good choice on a MacBook Pro;
  3840×2160 works too.
- *Live resolution changes*: **Off** (recommended) renders everything at full resolution, and
  changes to resolution or upscaler preset apply after *Apply and restart game* in the in-game
  menu. **On** applies them instantly, but the game's post-processing then stays at 1080p.
- *Fullscreen*, *Present mode* (FIFO is VSync; KosmicKrisp has no Mailbox), *Allow HDR*.

**Frame rate**
- *Mode*: **60** is recommended. 30 is the game's own rate; 60, 90 and *Unlocked* apply
  community frame-rate patches.
- *FPS limit*: 0 follows the display's refresh rate.

**Upscaler**
- *Upscaler*: **FSR 3.1** (recommended), TAA (cheaper, softer), MetalFX (experimental, no
  Native AA preset), or Off.
- *Preset*: **Native AA** renders at the output resolution and only anti-aliases (sharpest);
  Quality, Balanced and so on render smaller and upscale (faster).
- *Sharpening* and *Sharpness*: the upscaler's sharpening pass.
- *Object motion vectors*: keeps moving characters from smearing with a temporal upscaler.
  Leave it on.
- *Show FPS*: the frame-rate counter in the corner.

**Game effects**
- *Model detail*: **Highest** always uses the most detailed version of models, which reduces
  objects visibly switching detail as you approach them; *As in the game* is the original.
- Switches for chromatic aberration, depth of field, motion blur, SSAO, the game's own
  anti-aliasing, dynamic shadows, an optional SSR reflection effect, skipping the intro videos,
  a free camera and the debug menu.

**Performance**: leave these on their defaults unless asked to change them for testing.

**Mods**: put each mod in its own folder inside the mods folder (*Show in Finder* opens it). A
mod folder holds either `dvdroot_ps4`, or the game's folders (`chr`, `parts`, `map`, …)
directly. Turn mods on or off and change their order with the arrows; when two mods change the
same file, the lower one wins.

**Third-party patches**: shadPS4-format `.xml` patch files for version 01.09, from the patches
folder. Each patch can be switched on or off.

For **3840×1080 (32:9)** in the installed 0.4-v1 app, enable only the matching community
resolution patch, leave the launcher's output setting at 1920×1080, and turn **Upscaler** and
**Live resolution changes** off. The patch supplies the actual scene/UI size and camera aspect
ratio. The release's upscaling paths assume a 16:9 guest. The rebuilt renderer and shortcuts
below support independent 32:9 scene and UI coordinates.

High-resolution patches also enlarge the game's graphics heaps. The startup pipeline detects
those writes, including third-party patches, and raises direct memory from 5056 to **9152 MiB**.
An explicitly smaller `BB_DMEM_MB` stops with an explanation before the game starts. In the
**0.4-v1 release**, which predates this fix, set `BB_DMEM_MB=9152` under **Developer → Extra
variables** yourself; otherwise the 32:9 patch can crash during startup even on a 64 GB Mac.

From a checkout, `tools/macos/play_ultrawide.command` runs the installed release engine with
this fixed startup pipeline and the bundled 3840×1080 patch. It keeps separate settings/logs in
`out/ultrawide` and uses the installed app's saves. No source build is needed. Set `BB_USER_DIR`
to a copied saves folder for testing, or `BB_GAME_DIR` if the game folder has not been chosen in
the installed app. This shortcut uses the original model detail and disables DoF, motion blur
and dynamic light shadows as a conservative starting point. Gameplay performance at 32:9 still
needs measuring on each Mac; it has twice the pixels of 1920×1080.

After building `macos-native` with the 32:9 UI changes, double-click
`tools/macos/play_native_ultrawide.command`. Set `BB_NATIVE_CHECKOUT` to that checkout, or point
`out/native-checkout` at it. It needs both `out/bb-probe` and `out/gpu-arm64/bb-gpu`, with their
x86-64 and arm64 dependency prefixes. The renderer runs on Apple Silicon; the game's x86-64
code still runs through Rosetta. `tools/macos/build_m2_native.command` rebuilds the prepared
renderer with Release optimization, M2 instructions and link-time optimization.

The native shortcut now defaults to a **true 3840×1080 scene and UI**, with upscaling and
live resolution changes off. `tools/macos/play_full_resolution_ultrawide.command` explicitly
selects that profile. The older `play_fastest_ultrawide.command` now selects the same full
resolution. New settings in `out/native-fullres` use the game's original model detail, game AA
and SSAO; chromatic aberration, DoF, motion blur and dynamic light shadows are disabled. Existing
effect preferences are retained. The shortcuts use the installed app's normal saves; `BB_USER_DIR`
can select copied saves. Timing is recorded in that profile's `out/frames.csv`.

For this M2 Max setup, `tools/macos/play_m2_ultrawide.command` selects the tested
`macos-native-0.5` build through `out/native-05-checkout`. This interactive shortcut clears
scripted controller input, uses the normal saves folder, and keeps the scene and UI at
**3840×1080 with no upscaling**. Its separate `out/m2-ultrawide` profile starts with model
LOD 1, game AA and SSAO off; DoF, motion blur, chromatic aberration and dynamic light shadows
are also off. These are quality tradeoffs even though the rendering resolution stays full.
Existing preferences in this profile are retained.

This profile also enables **S = quicksave** and **L = quickload** while the game has
keyboard focus. These keys are reserved from their default backward-movement and D-pad-right
bindings; gamepad controls, including the rear-button R1 bindings, are retained. Settings/text
dialogs and modifier-key shortcuts do not trigger a checkpoint. Holding a key triggers once.

S requests a **fresh Bloodborne save of your current progress**, then copies it into a
separate checkpoint. It waits for new character and profile writes to finish and for all
save mounts and file handles to close. Wait for **“Quicksaved - L to load”** before moving on.
If the game cannot finish saving within 15 seconds, the previous checkpoint stays active
and an on-screen message explains that saving failed. This integration verifies the 1.09
save-request function before calling it; unrecognized game builds cannot create checkpoints.
L verifies the most recent successful checkpoint, restarts the port,
backs up the displaced progress, restores the checkpoint and advances Offline/Continue.
Automatic menu input stops at the character's writable save mount. This is a normal game
reload, not a frame-exact emulator state; there is a loading wait. Two copied-save tests on
this M2 Max returned to gameplay in about 30 seconds each. Press S before using L.

Checkpoints and pre-load backups live in
`~/Library/Application Support/bbport/user/quicksaves/CUSA00207/` (or the selected `BB_USER_DIR`).
Each save/load retains its own generation; these can be managed there when no game is running.
The active checkpoint is named in `current.json`; `before-load-*` holds displaced progress.
Do not replace these directories while playing. Restores use a recovery journal and verified
directory copies, so an interruption between directory renames is repaired on the next launch.
Set `BB_QUICKSAVE=0` to restore the standard S/L keyboard bindings. The native runtime changes
are in `tools/macos/patches/m2-quicksave.patch`; the helper is `tools/macos/quicksave.py`.

The optimized renderer skips attachmentless draws only when they have no shader writes,
motion-vector output or translated occlusion queries. One recording thread avoids splitting
render passes; two frames ahead and GPU QoS are enabled on this newer build. The older 0.4
build should keep one frame ahead because a prior two-frame test exposed malformed commands.

Measurements on this M2 Max (38 GPU cores, 64 GB, macOS 27.0.1) in the same loaded save:

| Configuration | Measured FPS |
|---|---:|
| Earlier native 0.4, original model detail/AA/SSAO | 29.2 |
| Native 0.5 with empty-draw fix, original model detail/AA/SSAO | 33.5 |
| Empty-draw fix, LOD 1, AA/SSAO off | 36.6 |
| Same tuned settings, unsplit render passes | 37.6 |

These are short stationary measurements after shader warmup, not a guarantee across the game.
The interactive session has also shown about 35 FPS over a 30-second interval. **Consistent
40+ FPS has not been achieved.** No generated frames are counted. Previously measured 47.8 FPS
at 1280×360 does not meet the required resolution and is not used by this shortcut.

The build also includes upstream memory-budget commit `de3f0606f7fc1b653174256d1a1984f39496f415`
(10 October 2026). It reserves room for memory outside the GPU's allocations instead of giving
the texture cache Metal's entire limit. A copied-save comparison at 3840×1080 measured 31.78 FPS
before and 31.97 FPS after: effectively unchanged in that scene. This is a memory-management
improvement, with no demonstrated FPS gain here. The author's new `--memory-import` diagnostic
passed on both arm64 and x86-64 for anonymous/shared buffers; the arm64 driver needs 16 KiB
alignment. Passing that buffer test does not validate the experimental whole-game memory model,
which remains off.

The working driver is the project's pinned KosmicKrisp version. An isolated newer-driver trial
regressed to about 24 FPS. Shared-memory experiments produced missing scenery and remain
explicitly disabled; their higher FPS numbers are invalid. Parked command-thread waits did not
improve the benchmark and are disabled too.

The tested source changes are preserved in `tools/macos/patches/m2-native-0.5.patch`, based on
native 0.5 commit `57b2dbde88`. They include the draw optimization, a cache-writer race fix,
mapped-region lookup caching, an input-thread idle fix, and a compute-submission ordering
fix for save-loading freezes. Compute work is enqueued before waiting for prior work to finish,
so a graphics queue waiting for that compute work can progress. The regression harness at
`tools/macos/test_compute_submit_order.py` reproduces the dependency with the original function
and verifies the repaired one. Apply this patch to a prepared
checkout of that commit, build its matching frontend/dependencies, then run
`BB_NATIVE_CHECKOUT=/path/to/checkout tools/macos/build_m2_native.command` to rebuild the
native renderer for M2. Set `BB_NATIVE_CHECKOUT` when launching, or point
`out/native-05-checkout` to that checkout. The full experimental diff is retained separately
under `out/native-tests`; it is not required for the tested profile.

**Developer**: switches for diagnostics (frame statistics or a GPU profile in the log), and
*Extra variables*: space-separated `NAME=value` settings for testing, for example
`BB_FILE_TRACE=1 BB_GPU_QOS=1`. Leave it empty unless you've been asked to set something.

## Recommended settings

On a recent MacBook Pro:

| Setting | Value |
|---|---|
| Output resolution | 2560×1440 |
| Live resolution changes | Off |
| Frame rate mode | 60 |
| Present mode | FIFO (VSync) |
| Upscaler | FSR 3.1, preset Native AA (or 3840×2160 with Quality or Balanced) |
| Model detail | Highest |

Turn **Low Power Mode off** (Battery in Control Centre, or System Settings ▸ Battery) and keep
the Mac plugged in: in Low Power Mode the game runs much slower.

## In the game

**The overlay menu** opens with **F1**, **`** or **§** on the keyboard, or **L3+R3** (both stick
buttons) on a controller. It has the upscaler and preset, sharpening, output resolution, the FPS
counter and the game effects. Changes to resolution or preset that need it are applied with
**Apply and restart game**, which restarts the game in place.

**Controllers**: a DualSense or other controller works over USB or Bluetooth. With several
connected, set `BB_GAMEPAD=` followed by part of the controller's name in *Extra variables*.

**Keyboard**:

| Keyboard | PS4 |
|---|---|
| W A S D | left stick |
| arrow keys | right stick |
| I J K L | D-pad |
| Space / Left Shift / E / Q | Cross / Circle / Square / Triangle |
| Enter | Options |
| 1 / 3 | L1 / R1 |
| R / F | L2 / R2 |
| Z / C | L3 / R3 |
| Tab / Backspace | touchpad |

**Remapping**: add lines to `bbport.ini` (Help ▸ Show the Data Folder), one per PS4 input, using
SDL's names for keys and controller buttons: for example `key.cross=Space` or `pad.circle=b`.
Several bindings for one input are separated by commas. Inputs without a line keep the defaults
above.

## Where your files are

The app writes nothing inside itself. Everything it keeps is in the **data folder**:

```
~/Library/Application Support/bbport/
    bbport.ini          game settings (also edited by the in-game menu)
    user/               saves and compiled shaders (unless you chose another saves folder)
    mods/, patches/     your mods and third-party patches
    out/last-run.log    everything the last run printed; out/previous-run.log is the one before
```

**Help ▸ Show the Data Folder** opens it in Finder. The launcher's own preferences (game folder,
switches) are kept by macOS for the app.

**Coming from a source build?** Your saves are in the `user/` folder of the checkout. Either
copy that folder into the data folder, or point *Saves folder* at it. A source checkout and the
app can share one saves folder, but don't run both at once.

## Updating

**Updates** in the app's window opens the Releases page. Download the new zip and replace
`bbport.app` in Applications with the new one; your settings and saves stay where they are.
After an update, the first launch compiles shaders again.

## Troubleshooting

- **The window stays black for a long time.** On the first launch, or after an update, this is
  the shader compilation: watch the Game Log, which keeps moving. If it stops for minutes with
  no new lines, see *Reporting a problem*.
- **It runs slowly.** Turn Low Power Mode off and plug the Mac in. Close other heavy apps. Busy
  areas run below 60 FPS on some Macs; 30 or a lower output resolution helps.
- **"Rosetta 2 seems to be missing" or "Python 3 seems to be missing"** under *Before you
  play*: run the matching command from [What you need](#what-you-need).
- **"expected a decrypted PS4 SELF or ELF"** in the log: the game files are still encrypted, or
  the folder isn't the merged 1.09 one.
- **"No eboot.bin in the folder"**: choose the folder that directly contains `eboot.bin`.
- **The game crashes or freezes**: note what you were doing, then see *Reporting a problem*. If
  it happens again at the same place, try *Two-stage GPU pipeline: Off* under Performance.
- **Some effects are missing.** Metal has no geometry shaders, so a few effects that need them
  aren't drawn yet (see *Known limitations* on the front page).

## Reporting a problem

Report problems with the Mac version on this project's
[Issues page](https://github.com/bmy/bbport-mac/issues) (Help ▸ Report a Problem…), not to
shadPS4 or to the upstream bbport project. Please include:

- your Mac model, macOS version and the app's version (bbport ▸ About bbport);
- what happened and where in the game;
- the log: **Help ▸ Show the Last Run's Log**, and attach `last-run.log` (if you've started the
  game again since, `previous-run.log` is the one from before).

## Building from source

Developers and testers can build everything from a checkout instead, including the experimental
branches (such as `macos-native`, which runs the renderer as a separate native arm64 process).
See [tools/macos/README-macos.md](../tools/macos/README-macos.md). A source build's app has an
**Update** button that fetches and rebuilds the branch set in bbport ▸ Settings.
