"""
python_autotune_demo.py
=======================

This script demonstrates how to perform basic pitch correction on a monophonic
vocals recording using Python. It relies on the librosa library for pitch
detection via the `pyin` algorithm and the psola package for pitch shifting.
The workflow follows the classic auto‑tune process: detect pitch, compute
the desired (correct) pitch according to a musical scale and shift the
original audio towards that pitch. See the Medium article and WolfSound
tutorials for background information.

Usage::

    python python_autotune_demo.py input.wav --scale C:maj --output output.wav

The script will load the input file, detect its fundamental frequency over
time, quantise those frequencies to the nearest note of the chosen scale and
apply pitch shifting so that the output is in tune. The result is saved to
`output.wav`. You can adjust the scale via the command line.

Note:
    This example requires the libraries librosa, numpy, scipy, soundfile and
    psola. Install them via pip::

        pip install librosa numpy scipy soundfile psola

    PSOLA (Pitch‑Synchronous Overlap and Add) is a time‑domain technique
    described in the literature for modifying pitch and timing without
    introducing significant artifacts. See the PSOLA Wikipedia article for a
    concise description of the algorithm【245150854923531†L117-L127】.
"""

import argparse
from typing import List
import numpy as np
import librosa
import scipy.signal as sig
import soundfile as sf
import psola


SEMITONES_IN_OCTAVE = 12

def parse_scale(scale_str: str) -> List[int]:
    """Return a list of scale degrees (MIDI note offsets within an octave) for a
    named scale. Supports major and minor. For example, C:maj returns
    [0, 2, 4, 5, 7, 9, 11]."""
    # parse key and mode
    key, mode = scale_str.split(":")
    # map note name to MIDI offset
    note_names = {"C": 0, "C#": 1, "Db": 1, "D": 2, "D#": 3, "Eb": 3,
                  "E": 4, "F": 5, "F#": 6, "Gb": 6, "G": 7, "G#": 8,
                  "Ab": 8, "A": 9, "A#": 10, "Bb": 10, "B": 11}
    root = note_names.get(key.upper(), 0)
    if mode.lower() == "maj" or mode.lower() == "major":
        degrees = [0, 2, 4, 5, 7, 9, 11]
    elif mode.lower() == "min" or mode.lower() == "minor":
        degrees = [0, 2, 3, 5, 7, 8, 10]
    else:
        # default to chromatic scale if unknown mode
        degrees = list(range(SEMITONES_IN_OCTAVE))
    # shift degrees by root and wrap around
    return [ (d + root) % SEMITONES_IN_OCTAVE for d in degrees ]


def get_closest_pitch(value: float, scale_degrees: List[int]) -> float:
    """Return the closest pitch (in Hz) on the given scale to the input value."""
    if np.isnan(value) or value <= 0.0:
        return np.nan
    # convert to MIDI note and fractional degree within octave
    midi_note = librosa.hz_to_midi(value)
    degree = midi_note % SEMITONES_IN_OCTAVE
    # prepare degrees extended by one octave to handle wrap around
    extended = np.array(scale_degrees + [scale_degrees[0] + SEMITONES_IN_OCTAVE], dtype=float)
    # find nearest degree index
    idx = np.argmin(np.abs(extended - degree))
    degree_diff = degree - extended[idx]
    corrected_midi = midi_note - degree_diff
    return librosa.midi_to_hz(corrected_midi)


def calculate_correct_pitch(f0: np.ndarray, scale_degrees: List[int]) -> np.ndarray:
    """Quantise an array of fundamental frequency estimates to the nearest scale note."""
    corrected = np.zeros_like(f0)
    for i, val in enumerate(f0):
        corrected[i] = get_closest_pitch(val, scale_degrees)
    # median filter to smooth over time
    corrected_med = sig.medfilt(corrected, kernel_size=11)
    corrected_med[np.isnan(corrected_med)] = corrected[np.isnan(corrected_med)]
    return corrected_med


def autotune_file(input_path: str, output_path: str, scale: str) -> None:
    """
    Perform auto‑tuning on a monophonic audio file. The result will be saved to
    the output path. The chosen scale should be provided in the format
    'C:maj' or 'A:min'.
    """
    # Load audio (mono)
    audio, sr = librosa.load(input_path, mono=True)
    # Estimate pitch using PYIN
    fmin = librosa.note_to_hz('C2')
    fmax = librosa.note_to_hz('C7')
    frame_length = 2048
    hop_length = frame_length // 4
    f0, voiced_flag, voiced_prob = librosa.pyin(audio,
                                               fmin=fmin,
                                               fmax=fmax,
                                               sr=sr,
                                               frame_length=frame_length,
                                               hop_length=hop_length)
    # Compute target pitch values
    scale_degrees = parse_scale(scale)
    corrected_f0 = calculate_correct_pitch(f0, scale_degrees)
    # Use PSOLA to resample to the corrected pitch
    pitch_shifted = psola.vocode(audio,
                                 sample_rate=int(sr),
                                 target_pitch=corrected_f0,
                                 fmin=fmin,
                                 fmax=fmax)
    # Write result
    sf.write(output_path, pitch_shifted, sr)


def main() -> None:
    parser = argparse.ArgumentParser(description="Simple auto‑tune utility using PYIN and PSOLA.")
    parser.add_argument("input", help="Path to input WAV file (monophonic)")
    parser.add_argument("--scale", default="C:maj", help="Musical scale (e.g. C:maj, A:min)")
    parser.add_argument("--output", default="output.wav", help="Path to output WAV file")
    args = parser.parse_args()
    autotune_file(args.input, args.output, args.scale)


if __name__ == "__main__":
    main()
