# Autotune Plugin (JUCE)

Vocal autotune plugin project for DAWs, implemented with JUCE.

## Fundamental process

Detect the singer's pitch, choose the closest target note, then shift the signal toward that target.

## What is inside

- `PluginProcessor.*`: Core plugin processing (pitch detect + shift + wet/dry mix)
- `YinPitchDetector.*`: Minimal YIN pitch detector
- `PluginEditor.h`: Optional custom editor skeleton (runtime currently uses JUCE generic editor)
- `python_autotune_demo.py`: Offline Python demo using `librosa.pyin` and `psola`

## Frameworks / libraries

- JUCE (plugin framework, realtime audio I/O and DSP utilities)
- CMake (build system)
- Python demo dependencies: `librosa`, `numpy`, `scipy`, `soundfile`, `psola`

## Build the plugin on macOS

Prerequisites:
- Xcode command line tools
- CMake 3.22+

Build:

```bash
cd /Users/ammar/Desktop/autotune_plugin
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

Build targets:
- AU
- VST3
- Standalone app

On first configure, CMake fetches JUCE from GitHub.

## Load in a DAW

1. Build in `Release`.
2. Open your DAW (Logic for AU, Ableton/REAPER/etc for VST3).
3. Rescan plugins in DAW preferences if needed.
4. Insert `Autotune Plugin` on a vocal track.
5. Use `Wet/Dry Mix` to blend corrected and original signal.

## Notes

- Educational implementation, not production-grade pitch correction.
- Currently snaps to nearest semitone (chromatic), not a selected musical key.

## Python demo

Install:

```bash
pip install librosa numpy scipy soundfile psola
```

Run:

```bash
python python_autotune_demo.py input.wav --scale C:maj --output output.wav
```
