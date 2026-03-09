/*
  ==============================================================================

    YinPitchDetector.cpp
    This file implements a minimal version of the YIN fundamental frequency
    estimator. The algorithm is based on difference functions and cumulative
    mean normalization and is suitable for monophonic pitch detection. See
    "YIN, a fundamental frequency estimator for speech and music" (De Cheveigné
    and Kawahara, 2002) for details.

  ==============================================================================
*/

#include "YinPitchDetector.h"

YinPitchDetector::YinPitchDetector (int sr, int bufSize, float thresh, float minFreq, float maxFreq)
    : sampleRate (sr), bufferSize (bufSize), threshold (thresh)
{
    // Preallocate buffers for difference function and CMND
    difference.resize (bufferSize / 2);
    cmnd.resize (bufferSize / 2);
    // Compute min and max lag (tau) based on min/max frequency
    tauMin = static_cast<int> (std::floor (static_cast<float> (sampleRate) / maxFreq));
    tauMax = static_cast<int> (std::ceil (static_cast<float> (sampleRate) / minFreq));
    if (tauMax > bufferSize / 2)
        tauMax = bufferSize / 2;
    if (tauMin < 2)
        tauMin = 2;
}

float YinPitchDetector::estimatePitch (const float* buffer)
{
    const int halfBuffer = bufferSize / 2;

    // 1. Difference function
    // d(tau) = sum_{j=0}^{N-1} (x[j] - x[j+tau])^2
    for (int tau = 0; tau < halfBuffer; ++tau)
    {
        double sum = 0.0;
        for (int j = 0; j < halfBuffer; ++j)
        {
            double diff = buffer[j] - buffer[j + tau];
            sum += diff * diff;
        }
        difference[tau] = static_cast<float> (sum);
    }

    // 2. Cumulative mean normalized difference (CMND)
    cmnd[0] = 1.0f;
    float runningSum = 0.0f;
    for (int tau = 1; tau < halfBuffer; ++tau)
    {
        runningSum += difference[tau];
        cmnd[tau] = difference[tau] * tau / runningSum;
    }

    // 3. Find the first local minimum below threshold
    int tauEstimate = -1;
    for (int tau = tauMin; tau <= tauMax; ++tau)
    {
        if (cmnd[tau] < threshold)
        {
            // search for minimum in the neighbourhood
            while (tau + 1 < halfBuffer && cmnd[tau + 1] < cmnd[tau])
                tau++;
            tauEstimate = tau;
            break;
        }
    }

    if (tauEstimate == -1)
        return 0.0f;

    // 4. Parabolic interpolation around tauEstimate to refine the peak
    int tau0 = tauEstimate;
    if (tau0 > 0 && tau0 + 1 < halfBuffer)
    {
        float s0 = cmnd[tau0 - 1];
        float s1 = cmnd[tau0];
        float s2 = cmnd[tau0 + 1];
        float denom = (2.0f * s1 - s0 - s2);
        if (std::abs (denom) > 1e-9f)
        {
            float delta = (s2 - s0) / (2.0f * denom);
            tau0 = tau0 + static_cast<int> (delta);
        }
    }

    // 5. Convert lag to frequency
    if (tau0 > 0)
        return static_cast<float> (sampleRate) / static_cast<float> (tau0);
    return 0.0f;
}
