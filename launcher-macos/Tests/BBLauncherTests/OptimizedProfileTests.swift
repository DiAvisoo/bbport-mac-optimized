import XCTest
@testable import BBLauncher

final class OptimizedProfileTests: XCTestCase {
    func testNativeWidePatchAndReturnToStandardResolution() {
        var prefs = LauncherPrefs()
        prefs.ultrawide = true
        prefs.resolution = "2560x1440"
        var game = GameSettings()
        game.upscaler = "fsr3"
        game.preset = 3
        var profile = OptimizedProfile(prefs: prefs, game: game)
        XCTAssertEqual(profile.settings.outputRes, "1920x1080")
        XCTAssertEqual(profile.settings.upscaler, "off")
        XCTAssertEqual(profile.environment["BB_PATCHES"], "Resolution Patch 3840x1080 (32:9)")
        XCTAssertEqual(profile.environment["BB_DMEM_MB"], "9152")
        XCTAssertNil(profile.environment["BB_RENDER_RES"])
        prefs.ultrawide = false
        profile = OptimizedProfile(prefs: prefs, game: game)
        XCTAssertEqual(profile.settings.outputRes, "2560x1440")
        XCTAssertEqual(profile.settings.upscaler, "fsr3")
        XCTAssertEqual(profile.environment["BB_PATCHES"], "")
        XCTAssertNil(profile.environment["BB_DMEM_MB"])
    }

    func testEveryWideOptionHasAnExactCommunityPatch() throws {
        let root = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
            .deletingLastPathComponent().deletingLastPathComponent().deletingLastPathComponent()
        let xml = try String(contentsOf: root.appendingPathComponent("patches/Bloodborne.xml"))
        for option in Catalog.wideResolutions {
            var prefs = LauncherPrefs()
            prefs.ultrawide = true
            prefs.wideResolution = option.value
            let profile = OptimizedProfile(prefs: prefs, game: GameSettings())
            let patch = try XCTUnwrap(profile.environment["BB_PATCHES"])
            XCTAssertTrue(xml.contains("Name=\"\(patch)\""), patch)
            XCTAssertEqual(profile.settings.upscaler, "off")
        }
    }

    func testQuicksaveIsOptInAndCanBeDisabledAgain() {
        var prefs = LauncherPrefs()
        XCTAssertEqual(OptimizedProfile(prefs: prefs, game: GameSettings()).environment["BB_QUICKSAVE"], "0")
        prefs.quicksave = true
        XCTAssertEqual(OptimizedProfile(prefs: prefs, game: GameSettings()).environment["BB_QUICKSAVE"], "1")
        prefs.quicksave = false
        XCTAssertEqual(OptimizedProfile(prefs: prefs, game: GameSettings()).environment["BB_QUICKSAVE"], "0")
    }

    func testDisplayAndSaveChoicesPersistAndInvalidSizesRecover() throws {
        let suite = "bbport.optimized.tests.\(UUID().uuidString)"
        let defaults = try XCTUnwrap(UserDefaults(suiteName: suite))
        defer { defaults.removePersistentDomain(forName: suite) }
        var prefs = LauncherPrefs()
        prefs.ultrawide = true
        prefs.wideResolution = "5120x1440"
        prefs.resolution = "2560x1440"
        prefs.quicksave = true
        prefs.save(to: defaults)
        XCTAssertEqual(LauncherPrefs.load(from: defaults), prefs)
        defaults.set("9000x500", forKey: "wideResolution")
        XCTAssertEqual(LauncherPrefs.load(from: defaults).wideResolution, "3840x1080")
    }

    func testInheritedBenchmarkAndOtherPortSettingsCannotLeak() {
        let clean = OptimizedProfile.cleanEnvironment([
            "HOME": "/home/test", "PATH": "/usr/bin", "BB_GPU_PROCESS": "1",
            "BB_QUICKLOAD_PENDING": "1", "BB_PAD_FILE": "/tmp/controller",
            "BB_DMEM_MB": "5056", "BB_QUICKSAVE": "1", "VK_DRIVER_FILES": "old-driver",
            "DYLD_LIBRARY_PATH": "other-port", "PYTHON": "other-python",
        ])
        XCTAssertEqual(clean, ["HOME": "/home/test", "PATH": "/usr/bin"])
    }

    func testIniPreservesControllerMappingsAndUnknownSettings() {
        let ini = IniFile(lines: ["pad.r1=custom", "unknown=keep", "output_res=3840x1080"], values: [:])
        let text = ini.text(applying: OptimizedProfile(prefs: LauncherPrefs(), game: GameSettings()).settings.iniValues)
        XCTAssertTrue(text.contains("pad.r1=custom\n"))
        XCTAssertTrue(text.contains("unknown=keep\n"))
        XCTAssertTrue(text.contains("output_res=1920x1080\n"))
    }
}
