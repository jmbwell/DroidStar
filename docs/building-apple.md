# Building DroidStar for macOS and iOS

Status: macOS build, launch, microphone permission, and DMR transmit/receive
verified. iOS signing, installation, launch, microphone permission, and DMR
transmit/receive are also verified on an iPhone 17 Pro Max. Other audio routes
and standalone macOS packaging are not verified.

## Prerequisites

- Xcode
- Qt 6.5 or newer
  - Use official installer: [Qt Online
  Installer](https://download.qt.io/official_releases/online_installers/)
  - Include macOS, iOS, Qt Multimedia, Qt Serial Bus, and Qt Serial Port modules
- CMake (`brew install cmake` or use what comes with Qt)


## macOS

### Configure

Run from the repository root. These commands were verified on Apple Silicon
with Qt 6.11.2, Xcode 26.6, and Homebrew CMake 4.4.3 on September 8, 2026.
Adjust the Qt installation path if using a different version or location.

```sh
"$HOME/Qt/6.11.2/macos/bin/qt-cmake" \
  -S . -B /tmp/droidstar-macos-build \
  -DUSE_MD380_VOCODER=AUTO \
  -DCMAKE_BUILD_TYPE=Debug
```

Use a fresh build directory for the first configuration. The `qt-cmake` wrapper
supplies Qt's toolchain and search paths; on the tested system it invoked
Homebrew CMake through `PATH`. Running plain `cmake` without a Qt search path
did not find the installation.

The tested configuration reported:

```text
-- Vocoder: bundled (MD380 header or library not found)
-- Configuring done
-- Generating done
```

If configuration reports missing Multimedia or SerialPort, add those components
for the macOS kit using Qt Maintenance Tool. A missing TaskTree dependency from
a QML plugin produced a warning in this installation but did not prevent
configuration or compilation.

### Build

```sh
cmake --build /tmp/droidstar-macos-build --parallel 6
```

The successful build ends with `Built target DroidStar` and produces:

```text
/tmp/droidstar-macos-build/DroidStar.app
```

This is an ARM64 debug development build that uses the installed Qt libraries.
It has not yet been packaged as a standalone application for distribution.
The build directory is temporary; for a persistent build, replace
`/tmp/droidstar-macos-build` with `build/macos` in both commands. Repeat the build
command after source changes.

### Runtime verification

Verified on September 8, 2026 with the Apple Silicon debug build:

- Application launch and the macOS microphone permission prompt.
- BrandMeister connection and a Parrot test using destination 310997 with Private checked.
- Smooth, intelligible transmit and receive audio using the bundled vocoder and
  the LG UltraFine Display microphone and speakers, selected through OS Default.
- Capture timing after conversion from native 48 kHz float audio to mono 8 kHz:
  6,010 ms elapsed produced 6,005 ms of audio, with 42 samples left queued.
  Playback accepted all supplied audio with no short writes.
- Rejected connections now report an error and stay disconnected instead of
  crashing or repeatedly reconnecting.

Other microphones, denied permission, device changes during a connection,
other digital modes, and standalone deployment still need live testing. 

## Apple app metadata

The app keeps the DroidStar name and existing artwork. CMake supplies version
`1.0.0` from the project version and a separate numeric build number. The Git
revision remains available for troubleshooting.

| CMake setting | Default | Purpose |
| --- | --- | --- |
| `DROIDSTAR_BUNDLE_IDENTIFIER` | `com.yourcompany.90d5dd37.DroidStar` | Preserves the existing identifier unless a builder overrides it. |
| `DROIDSTAR_BUILD_NUMBER` | `1` | Positive integer identifying an Apple build. Increment for subsequent distributed builds. |

Override the identifier for your own signed builds. Keep personal identifiers,
Qt paths, and signing-team settings in `CMakeUserPresets.json`, which Git ignores.
For example, with CMake 3.21 or newer:

```json
{
  "version": 3,
  "configurePresets": [{
    "name": "macos-local",
    "generator": "Unix Makefiles",
    "binaryDir": "${sourceDir}/build/macos",
    "cacheVariables": {
      "CMAKE_TOOLCHAIN_FILE": "$env{HOME}/Qt/6.11.2/macos/lib/cmake/Qt6/qt.toolchain.cmake",
      "CMAKE_BUILD_TYPE": "Debug",
      "DROIDSTAR_BUNDLE_IDENTIFIER": "com.example.DroidStar",
      "DROIDSTAR_BUILD_NUMBER": "1"
    }
  }],
  "buildPresets": [{
    "name": "macos-local",
    "configurePreset": "macos-local",
    "jobs": 6
  }]
}
```

Replace the example identifier and Qt path with your own, then run:

```sh
cmake --preset macos-local
cmake --build --preset macos-local
```

This produces `build/macos/DroidStar.app`. No personal identifier or signing
team is required in the shared project files. For iOS, use the iOS Qt toolchain
and the Xcode generator in a separate preset/build directory. Set
`CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM` locally when configuring device signing.

macOS uses the project's plist template. iOS retains Qt's generated plist and
default launch storyboard, adds the microphone explanation, and uses the
existing AppIcon asset catalog. Both platforms use this permission text:
"DroidStar uses your microphone to transmit your voice to amateur radio networks."

## iOS

Use the Qt iOS kit's `qt-cmake` wrapper to configure an Xcode project, following
the [Qt iOS build workflow](https://doc.qt.io/qt-6/ios.html), or use a local preset
as described above. Xcode project generation was verified with Qt 6.11.2 and
Xcode 26.6. An unsigned arm64 device build was verified with an iOS 17 minimum
deployment target. The iOS build embeds the FFmpeg frameworks required by the
prebuilt Qt Multimedia package using `qt_add_ios_ffmpeg_libraries()` when that
helper is available. See [Qt's iOS multimedia notes](https://doc.qt.io/qt-6/qtmultimedia-apple.html).

The launch-screen compiler also needs a matching simulator runtime installed
in Xcode, even for a device build. With the tested Xcode installation, installing
iOS 26.5 alone left an SDK/runtime build mismatch. The following local mapping
resolved `iOS 26.5 Platform Not Installed`:

```sh
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
xcodebuild -downloadPlatform iOS -buildVersion 26.5 -architectureVariant arm64
xcrun simctl runtime match set iphoneos26.5 23F73 --sdkBuild 23F81a
```

These build numbers are specific to the tested installation. Inspect
`xcrun simctl runtime list` and `xcrun simctl runtime match list` before applying
a mapping to another Xcode version. To clear this override, use
`xcrun simctl runtime match set iphoneos26.5 --default --sdkBuild 23F81a`.

For physical-device testing, connect and trust the iPhone, enable Developer Mode
in its Privacy & Security settings, and add the Apple account in Xcode Settings.
Select a development team for the DroidStar target and keep that team in the
ignored local preset so regeneration preserves it. Apple may require acceptance
of an updated developer agreement before automatic provisioning can succeed.

The signed Debug build was installed on an iPhone 17 Pro Max running iOS 26.6.1.
The resulting app's identifier, version/build fields, microphone explanation,
launch screen, and icon were checked, and its code signature passed verification.
Using the local `ios-local` preset described above, the command-line workflow is:

```sh
cmake --preset ios-local
xcodebuild -project build/ios/DroidStar.xcodeproj -target DroidStar \
  -configuration Debug -sdk iphoneos -allowProvisioningUpdates \
  -allowProvisioningDeviceRegistration -jobs 6
xcrun devicectl list devices
xcrun devicectl device install app --device <device-identifier> \
  build/ios/Debug-iphoneos/DroidStar.app
xcrun devicectl device process launch --device <device-identifier> <bundle-identifier>
```

### Runtime verification

Verified on September 8, 2026 with the signed Debug build on an iPhone 17 Pro Max
running iOS 26.6.1:

- Application launch, microphone permission, and BrandMeister BM_3102 connection.
- Transmit and receive using the bundled vocoder, built-in iPhone microphone,
  and speaker. Parrot destination **310997 requires Private checked**; group
  calls to that destination produced no return audio in this test.
- Capture used the active iOS audio session's 48 kHz rate and mono Int16 format.
  A 6,426 ms transmission produced 6,440 ms of converted 8 kHz audio, within the
  session's 23 ms I/O callback duration. One 20 ms frame remained queued at stop.
- Playback supplied, accepted, and processed all 6,300 ms of returned audio,
  with zero short writes and no bytes left buffered or pending. The tester
  reported substantially improved audio.

The initial build used Qt's fixed 44.1 kHz preferred-format default on iOS and
delivered only about 70% of elapsed capture time. The tested follow-up queries
the active audio session rate, allows 250 ms of capture buffering, retries
pending playback data, and drains the final audio before stopping. This test
confirms normal timing for the built-in route; it does not isolate the effect
of each change or verify other audio routes.

Bluetooth and wired audio, route changes during a connection, denied microphone
permission, application interruptions, other digital modes, and simulator
execution still need testing.

USB AMBE devices and MMDVM modems are excluded from the existing iOS build.

## Vocoder selection

`USE_MD380_VOCODER` defaults to `AUTO`. CMake looks for the MD380 header and
library and checks that the required API compiles and links for the selected
build target. If either is missing or the check fails, it uses the bundled codecs.
The configure output reports which implementation was selected and why.

- `-DUSE_MD380_VOCODER=AUTO`: automatic selection, including when restoring the
  default in a build directory previously configured with `ON` or `OFF`.
- `-DUSE_MD380_VOCODER=ON`: require MD380; configuration fails if it cannot be used.
- `-DUSE_MD380_VOCODER=OFF`: always use the bundled codecs.

For a nonstandard installation, set `CMAKE_PREFIX_PATH` to its prefix or set
`MD380_VOCODER_INCLUDE_DIR` and `MD380_VOCODER_LIBRARY` explicitly. The Raspberry
Pi library is not compatible with Apple targets; enabling this option does not
port it. The check compiles and links but does not execute code, so runtime
behavior and audio quality still require testing on the target device.

## Known gaps in this checkout

- The default bundle identifier is retained for upstream compatibility; builders
  should supply their own identifier for signed distribution. iOS bundle metadata
  and signing are verified; visual layout across device sizes remains unverified.
- Microphone permission prompts and DMR audio work on the tested macOS and iOS
  devices. Denied permission handling, other audio routes, route changes, and
  application interruptions remain to be verified.
- The deployment configuration uses the generic Qt application helper. It needs
  review for packaging this QML application on macOS
- Some README instructions refer to `DroidStar.pro`, which is absent from this
  CMake-based checkout
- There is currently no dependency installation or setup script in the
  repository, which might be something we want to put together for macOS and iOS
