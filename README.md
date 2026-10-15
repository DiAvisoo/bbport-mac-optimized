# bbport-mac-optimized

An optimized Apple Silicon build of [bmy/bbport-mac](https://github.com/bmy/bbport-mac), with a native GPU process, ultrawide support, a simplified launcher, and optional quicksave/quickload.

**Tested: 30–40 FPS at true 3840×1080 (32:9) on a Mac Studio M2 Max, 64 GB RAM.** This is a user observation, not a performance guarantee. Other Macs, including M1, are untested. The renderer is native arm64; the game's x86-64 code runs through Rosetta 2.

- [Installation and controls](docs/USER_GUIDE.md)
- [0.1.0 preview notes](docs/release/0.1.0.md)
- [Build and packaging instructions](docs/release/BUILD-OPTIMIZED.md)
- [Releases](https://github.com/DiAvisoo/bbport-mac-optimized/releases) · [Issues](https://github.com/DiAvisoo/bbport-mac-optimized/issues)

## Play

Download and extract the app archive, move **Bloodborne Optimized.app** to Applications, choose your game folder, and press **Play**. Requires macOS 26+, Apple Silicon and Rosetta 2. The app bundles its runtime and libraries; no compiling, Homebrew or separate Python installation is needed.

**Supply your own decrypted Bloodborne game files with update 1.09 merged in. No game files, artwork, saves, or shader caches are distributed.** This project is not affiliated with Sony, FromSoftware, Apple, AMD, or shadPS4.

Choose resolution and ultrawide directly in the launcher. Quicksave/quickload is optional, with click-to-remap shortcuts: **F1** configuration, **F2** quicksave, **F3** quickload. Extra settings are in **Advanced**. Existing bbport saves are shared by default; choose a copied save folder for isolated testing.

Graphical glitches after loading or changing resolution often clear after restarting the game. An occasional startup failure has also been seen at 720p. This preview is ad-hoc signed and is not notarized by Apple.

## Changes and credits

The integrated source is based on the `macos-native-0.5` engine at `57b2dbd`, plus the optimized launcher/patch compiler originally developed on `macos-0.4` at `5c7b9c8`. Changes include corrected ultrawide memory sizing, cached guest-memory region lookup, empty-draw skipping, compute submission ordering, shader-cache queue safety, controller/input fixes, and fresh-save snapshots with safe restoration.

Built on the work of bmy, deadinside28's [bbport](https://github.com/deadinside28/bloodborne_pc), shadPS4, KosmicKrisp/Mesa, and their contributors. Community resolution patches retain their author metadata. See [LICENSE](LICENSE), the app's third-party notices, and the matching source archive distributed with each release. The [upstream README](docs/UPSTREAM-README.md) retains the original technical background.
