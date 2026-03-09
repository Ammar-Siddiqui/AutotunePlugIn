/*
  ==============================================================================

    YinPitchDetector.h
    Simple implementation of the YIN pitch detection algorithm. Based on the
    description from the original YIN paper and various educational sources,
    YIN estimates the fundamental frequency of a signal by computing the
    difference function and cumulative mean normalized difference (CMND)
    and selecting the first local minimum under a threshold. For details see
    the algorithm overview in the documentation.

  ==============================================================================
*/

#pragma once

#include <vector>
#include <cmath>

/**
    A basic implementation of the YIN pitch detection algorithm.

    Usage:
        YinPitchDetector detector (sampleRate, bufferSize);
        float pitchHz = detector.estimatePitch (inputBuffer);

    The input buffer should contain bufferSize samples of monophonic audio.
    The detector will return a positive frequency in Hz if a pitch was
    detected or 0.0f if no pitch was found (e.g. unvoiced, silence or noise).
*/
class YinPitchDetector
{
public:
    YinPitchDetector (int sampleRate, int bufferSize, float threshold = 0.15f, float minFreq = 80.0f, float maxFreq = 1000.0f);
    ~YinPitchDetector() = default;

    /** Estimates the fundamental frequency of the provided audio block.
        @param buffer Pointer to bufferSize samples of monophonic audio.
        @returns Detected frequency in Hertz or 0.0f if no fundamental was found.
    */
    float estimatePitch (const float* buffer);

private:
    int sampleRate;
    int bufferSize;
    float threshold;
    int tauMin;
    int tauMax;

    std::vector<float> difference;
    std::vector<float> cmnd;
};
