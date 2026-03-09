# Autotune Plugin (JUCE)

This repository contains a basic real-time autotune-style audio plugin.

## What is inside

- `PluginProcessor.*`: Core plugin audio processing (pitch detect + shift + mix).
- `YinPitchDetector.*`: Minimal YIN pitch detector.
- `PluginEditor.h`: Optional custom editor skeleton (current build uses JUCE generic editor).
- `python_autotune_demo.py`: Offline Python demo using `librosa.pyin` and `psola`.

## Build the plugin on macOS

Prerequisites:
- Xcode command line tools
- CMake 3.22+

Build steps:

```bash
cd /Users/ammar/Desktop/autotune_plugin
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

The CMake setup builds these plugin formats:
- AU
- VST3
- Standalone app

On first configure, CMake will fetch JUCE from GitHub.

## Load it in a DAW

1. Build in `Release`.
2. Open your DAW (Logic for AU, Ableton/REAPER/etc for VST3).
3. Rescan plugins in DAW preferences if needed.
4. Insert `Autotune Plugin` on a vocal track.
5. Use the `Wet/Dry Mix` control to blend corrected and original signal.

## Notes

- This is an educational autotune implementation, not a production-grade pitch correction engine.
- It snaps to the nearest semitone (chromatic), not a selected musical key.

## Python demo

Install dependencies:

```bash
pip install librosa numpy scipy soundfile psola
```

Run:

```bash
python python_autotune_demo.py input.wav --scale C:maj --output output.wav
```
