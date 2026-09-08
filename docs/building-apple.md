# Building DroidStar for macOS and iOS

Status: initial setup notes

## Prerequisites

- Xcode
- Qt 6.5 or newer
  - Use official installer: [Qt Online
  Installer](https://download.qt.io/official_releases/online_installers/)
  - Include macOS, iOS, Qt Multimedia, and Qt Serial Bus modules
- CMake (`brew install cmake` or use what comes with Qt)


## macOS

The CMake project declares an Apple application bundle and links AVFoundation.
The next milestone is a local `.app` that launches and supports microphone
input, playback, and a network connection.

## iOS

Use the Qt iOS kit's `qt-cmake` wrapper to configure an Xcode project, following
the [Qt iOS build workflow](https://doc.qt.io/qt-6/ios.html). Exact commands,
simulator compatibility, and signing configuration remain to be verified.

Test on a simulator where supported, then on a physical device with signing
configured in Xcode. Device testing needs to cover microphone permissions,
capture and playback, audio route changes, and application interruptions.

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

- The bundled codec paths need build and audio validation on Apple platforms.
- `Info.plist` contains a placeholder bundle identifier and microphone usage
  description, and refers to a launch screen that is absent from the checkout.
  Need to set a proper bundle identifier, add a launch screen, and verify the
  microphone usage description
- The explicit microphone permission request currently runs only on Android.
  Apple permission handling needs implementation
- The deployment configuration uses the generic Qt application helper. It needs
  review for packaging this QML application on macOS
- Some README instructions refer to `DroidStar.pro`, which is absent from this
  CMake-based checkout
- There is currently no dependency installation or setup script in the
  repository, which might be something we want to put together for macOS and iOS
