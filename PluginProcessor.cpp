/*
  ==============================================================================

    PluginProcessor.cpp
    Created for the example autotune plugin. Implements basic pitch detection
    using YIN and pitch shifting via resampling. For educational purposes only.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
AutotuneAudioProcessor::AutotuneAudioProcessor()
    : parameters (*this, nullptr, juce::Identifier("AutotuneParameters"),
                  {
                      std::make_unique<juce::AudioParameterFloat> (paramMix,
                                                                     "Wet/Dry Mix",
                                                                     juce::NormalisableRange<float> (0.0f, 1.0f),
                                                                     0.5f)
                  })
{
}

AutotuneAudioProcessor::~AutotuneAudioProcessor()
{
}

//==============================================================================
const juce::String AutotuneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AutotuneAudioProcessor::acceptsMidi() const
{
   return false;
}

bool AutotuneAudioProcessor::producesMidi() const
{
    return false;
}

bool AutotuneAudioProcessor::isMidiEffect() const
{
    return false;
}

double AutotuneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AutotuneAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AutotuneAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AutotuneAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AutotuneAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AutotuneAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AutotuneAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    const int targetBufferSizeMs = 40; // analysis window roughly 40ms
    yinBufferSize = static_cast<int> (newSampleRate * targetBufferSizeMs / 1000.0);
    // ensure even size
    if (yinBufferSize % 2 != 0)
        yinBufferSize++;
    analysisBuffer.clear();
    analysisBuffer.resize (yinBufferSize);
    analysisBufferPos = 0;
    smoothedPitch = 0.0f;
    previousRatio = 1.0f;

    yinDetector = std::make_unique<YinPitchDetector> (static_cast<int> (newSampleRate), yinBufferSize);

    // allocate interpolators and temporary buffers for each channel
    const int numChannels = getTotalNumInputChannels();
    interpolators.clear();
    tempBuffers.clear();
    interpolators.resize (numChannels);
    tempBuffers.resize (numChannels);
    for (int c = 0; c < numChannels; ++c)
    {
        interpolators[c].reset();
        tempBuffers[c].setSize (1, samplesPerBlock);
    }
}

void AutotuneAudioProcessor::releaseResources()
{
    // Nothing to release
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AutotuneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This plug‑in works as a mono or stereo insert; simply ensure the number of
    // input and output channels matches and is mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}
#endif

//==============================================================================
float AutotuneAudioProcessor::computeNearestSemitone (float freq) const
{
    if (freq <= 0.0f)
        return 0.0f;
    // Convert to MIDI note
    float midi = 69.0f + 12.0f * std::log2 (freq / 440.0f);
    float nearestMidi = std::round (midi);
    // Convert back to frequency
    float nearestFreq = 440.0f * std::pow (2.0f, (nearestMidi - 69.0f) / 12.0f);
    return nearestFreq;
}

void AutotuneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    auto* mixParam = parameters.getRawParameterValue (paramMix);
    const float mix = mixParam != nullptr ? *mixParam : 0.5f;

    // Feed samples into analysis buffer (use first channel only)
    const float* inputData = buffer.getReadPointer (0);
    for (int i = 0; i < numSamples; ++i)
    {
        analysisBuffer[analysisBufferPos] = inputData[i];
        analysisBufferPos = (analysisBufferPos + 1) % yinBufferSize;
    }

    // Once we have filled the buffer, perform pitch detection
    float currentPitch = 0.0f;
    if (yinDetector != nullptr)
    {
        // Build a continuous segment for detection by copying from the ring buffer
        std::vector<float> window (yinBufferSize);
        int writePos = analysisBufferPos;
        for (int j = 0; j < yinBufferSize; ++j)
        {
            int idx = (writePos + j) % yinBufferSize;
            window[j] = analysisBuffer[idx];
        }
        currentPitch = yinDetector->estimatePitch (window.data());
    }

    if (currentPitch > 0.0f && currentPitch < 5000.0f) // ignore unrealistic pitches
    {
        // Exponential smoothing of detected pitch to avoid abrupt changes
        if (smoothedPitch <= 0.0f)
            smoothedPitch = currentPitch;
        else
            smoothedPitch = 0.95f * smoothedPitch + 0.05f * currentPitch;

        detectedPitchHz = smoothedPitch;
    }

    // Determine target frequency and pitch ratio
    float targetFreq = computeNearestSemitone (detectedPitchHz);
    float ratio = 1.0f;
    if (detectedPitchHz > 0.0f && targetFreq > 0.0f)
        ratio = targetFreq / detectedPitchHz;

    // Smooth ratio to avoid rapid jumps
    float smoothedRatio = 0.95f * previousRatio + 0.05f * ratio;
    previousRatio = smoothedRatio;

    // Process each channel: copy input to temp buffer then apply interpolator
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto* tempData    = tempBuffers[channel].getWritePointer (0);
        // Copy input to temp buffer
        std::memcpy (tempData, channelData, sizeof (float) * (size_t) numSamples);
        // Clear the output
        std::fill (channelData, channelData + numSamples, 0.0f);
        // Process pitch shift
        auto& interp = interpolators[channel];
        int consumed = interp.process (smoothedRatio, tempData, channelData, numSamples);
        juce::ignoreUnused (consumed);
    }

    // Mix dry and wet signal
    const float dryLevel = 1.0f - mix;
    const float wetLevel = mix;
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto* inputCopy   = tempBuffers[channel].getReadPointer (0);
        for (int i = 0; i < numSamples; ++i)
        {
            float wet = channelData[i];
            float dry = inputCopy[i];
            channelData[i] = dryLevel * dry + wetLevel * wet;
        }
    }
}

//==============================================================================
bool AutotuneAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AutotuneAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void AutotuneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Store parameter state
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AutotuneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml.get() != nullptr)
    {
        if (xml->hasTagName (parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin...
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutotuneAudioProcessor();
}
