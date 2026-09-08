# Audio capture conversion tests

Run from the repository root, adjusting the Qt installation path as needed:

```sh
"$HOME/Qt/6.11.2/macos/bin/qt-cmake" \
  -S tests/audio-capture -B build/audio-capture-tests \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/audio-capture-tests
ctest --test-dir build/audio-capture-tests --output-on-failure
```

These tests generate PCM in memory; they do not access microphones, speakers,
credentials, or radio networks. They check duration and pitch at 8, 44.1, 48,
and 96 kHz; arbitrary callback boundaries (including partial frames); reset
behavior; stereo downmix; anti-alias filtering; and Int16 levels.

Live verification is separate: compare the Log tab's elapsed capture time with
converted sample duration and native frame count after a short Parrot call.
At 48 kHz, five seconds should yield approximately 240,000 native frames and
40,000 converted samples, with only a small queue at the end of transmission.
