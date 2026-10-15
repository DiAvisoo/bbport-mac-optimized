# Rebuilding the optimized release

Source lineage: native engine at bmy/bbport-mac `macos-native-0.5` commit `57b2dbde8888ac50a7e742c0987df52e1bb11ac6`, combined with the tested launcher and patch compiler from `macos-0.4` commit `5c7b9c87d70c61ecf3794a062af4d32a1291c437`. The original frontend edits are preserved in local commit `17e3145`; this branch contains the integrated runtime source, including quicksave and remapping.

On Apple Silicon with Xcode/Command Line Tools and Homebrew:

```sh
git submodule update --init --recursive
bash tools/macos/setup_deps.sh
BB_ARCH=arm64 bash tools/macos/setup_deps.sh
# Use an installed SDK supported by your compiler/linker.
export SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"
bash tools/macos/build.sh
bash tools/macos/build_m2_native.command
swift test --package-path launcher-macos
bash tools/macos/package.sh 0.1.0
```

`build_m2_native.command` uses Release, LTO, and `-O3 -mcpu=apple-m2`. The release was built with the macOS 26.5 SDK. Runtime defaults are in `OptimizedProfile.swift` and `run_optimized.sh`. For a standalone source build, the launcher now reads binaries from this same checkout's `out/` directory.

The packager accepts `BB_PACKAGE_NATIVE` to select an existing build of these exact engine sources, and `BB_PACKAGE_PYTHON` to select a Python 3.11 framework prefix. It refuses mismatched runtime source, absolute non-system library loads, missing dependencies, binaries requiring macOS newer than 26, and prohibited package files. It copies only explicit engine scripts, community patches, executable code, licenses, and Python's standard library. Python extensions needing external libraries are omitted: these internal preparation/save tools only need the standard system-linked modules.

The source archive includes tracked project/submodule sources and tracked dependency sources with their local patches applied; build outputs and private user data are excluded. Its manifest records dependency revisions. System SDK files and tools are not redistributed. The release reuses the exact tested runtime binaries; their input hashes are recorded before relocation and signing.

The packager generates the app ZIP, local source export, audit and checksums. The public release uploads only the app ZIP; publish the matching source tag and retain the dependency revisions and patches documented in [SOURCE.md](SOURCE.md). Include the ZIP checksum in the release notes. The app is ad-hoc signed, not notarized. The build workflow produces artifacts; publishing is a separate step.
