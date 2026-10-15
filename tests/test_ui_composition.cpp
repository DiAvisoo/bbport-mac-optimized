// SPDX-License-Identifier: GPL-2.0-or-later
#include <cassert>
#include <cstdio>
#include "gpu/shadps4/video_core/renderer_vulkan/ui_composition.h"

int main() {
    using namespace Vulkan::UiComposition;
    // Title/loading menus have no depth/camera. UI must still go to the native target.
    assert(Choose(true, true, false, true) == Background::Copy);
    assert(Choose(true, true, false, false) == Background::Copy);
    // In-game HUD: first reconstruct the scene, then draw the native UI over it.
    assert(Choose(true, true, true, true) == Background::Temporal);
    // Runtime FSR disable/failure must not change UI resolution.
    assert(Choose(true, true, true, false) == Background::Copy);
    assert(Choose(true, false, true, true) == Background::None);
    assert(Choose(false, true, true, true) == Background::None);
    assert(NativeViewport(1920, -1080));
    assert(!NativeViewport(960, -540));
    assert(MovieShader(0x34e8a281) && MovieShader(0x24042a9b));
    assert(!MovieShader(0x0b0acf50)); // fullscreen post/tonemap also has a native viewport
    assert(!MovieShader(0x6c62a79f)); // shadow geometry
    const auto display = Scale(960, 540, 1920, 1080, false);
    assert(display[0] == 2 && display[1] == 2);
    const auto ui = Scale(960, 540, 1920, 1080, true);
    assert(ui[0] == 1 && ui[1] == 1); // don't double an already native UI viewport
    const auto larger = Scale(960, 540, 3840, 2160, true);
    assert(larger[0] == 2 && larger[1] == 2);
    // At 32:9 the UI uses 3840x1080 coordinates over a 2560x720 scene. Native text and
    // masks retain their size; scene/display coordinates scale uniformly by 1.5.
    assert(NativeViewport(3840, -1080, 3840, 1080));
    assert(!NativeViewport(1920, -1080, 3840, 1080));
    const auto wide_ui = Scale(2560, 720, 3840, 1080, true, 3840, 1080);
    assert(wide_ui[0] == 1 && wide_ui[1] == 1);
    const auto wide_scene = Scale(2560, 720, 3840, 1080, false, 3840, 1080);
    assert(wide_scene[0] == 1.5f && wide_scene[1] == 1.5f);
    std::puts("UI composition: PASS (menu without camera, HUD, FSR off, viewport scaling)");
}
