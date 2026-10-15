// SPDX-License-Identifier: GPL-2.0-or-later
// bbport launcher for macOS: picks the game folder, edits bbport.ini and the start-up
// environment, and runs tools/macos/run.sh (the Mac counterpart of launcher/bbport_launcher.py).
import AppKit
import SwiftUI

@main
struct BBLauncherApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) private var appDelegate

    var body: some Scene {
        Window("Bloodborne", id: "main") {
            ContentView()
                .environmentObject(LauncherModel.shared)
        }
        .defaultSize(width: 680, height: 860)
        .commands {
            CommandGroup(replacing: .help) {
                HelpCommands()
            }
        }

        Window("Game Log", id: "log") {
            LogView()
                .environmentObject(LauncherModel.shared)
        }
        .defaultSize(width: 900, height: 560)

        Settings {
            LauncherSettingsView()
                .environmentObject(LauncherModel.shared)
        }
    }
}

@MainActor
final class AppDelegate: NSObject, NSApplicationDelegate {
    func applicationDidFinishLaunching(_ notification: Notification) {
        // Also come to the front when started as a bare executable (swift run).
        NSApplication.shared.setActivationPolicy(.regular)
        NSApplication.shared.activate()
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }

    func applicationWillTerminate(_ notification: Notification) {
        LauncherModel.shared.prepareForQuit()
    }
}

/// Help menu: the user guide, downloads, problem reports and the last log.
struct HelpCommands: View {
    var body: some View {
        Button("bbport User Guide") { Links.open(Links.userGuide) }
        Button("Downloads and Release Notes") { Links.open(Links.releases) }
        Button("Report a Problem…") { Links.open(Links.issues) }
        Divider()
        Button("Show the Last Run's Log") { LauncherModel.shared.revealLastLog() }
        Button("Show the Data Folder") { LauncherModel.shared.reveal(LauncherModel.shared.dataURL) }
    }
}

/// Settings (⌘,): which game engine runs (the release app's own, or a checkout).
struct LauncherSettingsView: View {
    @EnvironmentObject var model: LauncherModel

    var body: some View {
        Form {
            Section {
                if model.engineURL != nil {
                    LabeledContent("Built into this app") {
                        Text(model.appVersion.isEmpty ? "Yes" : "Version \(model.appVersion)")
                    }
                } else {
                    LabeledContent("Embedded at build") {
                        Text(model.embeddedRepoPath ?? "None")
                            .textSelection(.enabled)
                            .truncationMode(.middle)
                    }
                }
                TextField(text: $model.prefs.repoOverride,
                          prompt: Text(model.engineURL != nil ? "Use the built-in engine" : "Use the embedded path")) {
                    Text("Checkout")
                }
                HStack {
                    Button("Choose…") { model.chooseRepository() }
                    Button(model.engineURL != nil ? "Use the Built-in Engine" : "Use the Embedded Path") {
                        model.prefs.repoOverride = ""
                    }
                    .disabled(model.prefs.repoOverride.isEmpty)
                }
                CheckLabel(check: model.repoCheck)
                LabeledContent("Data folder") {
                    Text(model.dataURL.path)
                        .textSelection(.enabled)
                        .truncationMode(.middle)
                }
            } header: {
                Text("Game engine")
            } footer: {
                Text(model.engineURL != nil
                     ? "This app carries its own engine; settings, saves and logs go to the data folder. To use an optimized source checkout instead, choose its folder."
                     : "The optimized source checkout with its native build in out/. The app starts the prebuilt native renderer without recompiling.")
                    .foregroundStyle(.secondary)
            }

        }
        .formStyle(.grouped)
        .frame(width: 580, height: 400)
    }
}
