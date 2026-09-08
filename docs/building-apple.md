# Building DroidStar for macOS and iOS

Status: initial setup notes

## Prerequisites

- Xcode
- CMake (`brew install cmake` or [official
  download](https://cmake.org/download/))
- Qt 6.5 or newer (Use official installer: [Qt Online
  Installer](https://download.qt.io/official_releases/online_installers/))


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

## Known gaps in this checkout

- `CMakeLists.txt` unconditionally enables the external `md380_vocoder` library,
  which is not included, and current builds are not for Apple systems anyway.
  Make it an explicit optional dependency and validate the bundled codec paths
  before documenting a default build command.
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
