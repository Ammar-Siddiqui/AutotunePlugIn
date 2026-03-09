/*
  ==============================================================================

    This file contains the basic framework code for a JUCE audio plug‑in that
    performs simple pitch correction (Auto‑Tune) in real time. The processor
    detects the fundamental frequency of the incoming audio using a small
    implementation of the YIN algorithm and then shifts the pitch towards the
    nearest semitone. Pitch shifting is performed using JUCE's built‑in
    LagrangeInterpolator which resamples the audio to achieve a pitch shift
    without significantly changing the buffer length. The result is blended
    with the original signal to allow for a wet/dry mix.

    NOTE: This code is intended as an educational example. Professional
    autotune plug‑ins employ more sophisticated pitch detection and phase
    alignment techniques (for example PSOLA or phase‑vocoder methods) to
    minimise artifacts. However, this example demonstrates the core concepts
    required to build a working pitch correction effect.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "YinPitchDetector.h"

//==============================================================================
/**
    An audio processor that performs pitch detection and pitch shifting.

    The processor runs a simple YIN‑based pitch detector on a sliding window
    of the input audio stream, computes the nearest note on the chromatic
    scale, calculates the pitch‑shift ratio required to correct the detected
    frequency and then uses a LagrangeInterpolator to resample the audio and
    apply the correction. The processor exposes a wet/dry mix parameter to
    control the amount of processed signal mixed with the original.
*/
class AutotuneAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    AutotuneAudioProcessor();
    ~AutotuneAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#if ! JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;

    // Parameter identifiers
    static constexpr const char* paramMix = "mix";

    // Pitch detection
    std::unique_ptr<YinPitchDetector> yinDetector;
    std::vector<float> analysisBuffer;
    int analysisBufferPos = 0;
    int yinBufferSize = 0;
    float detectedPitchHz = 0.0f;
    float smoothedPitch = 0.0f;
    float previousRatio = 1.0f;

    // Pitch shifting
    std::vector<juce::LagrangeInterpolator> interpolators;
    std::vector<juce::AudioBuffer<float>> tempBuffers;

    // Compute nearest semitone frequency (chromatic scale)
    float computeNearestSemitone (float freq) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutotuneAudioProcessor)
};
