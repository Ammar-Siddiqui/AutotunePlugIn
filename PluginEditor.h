/*
  ==============================================================================

    PluginEditor.h
    Created: to provide a user interface for the AutotuneAudioProcessor.

    In this example we rely on the generic editor provided by JUCE, which
    automatically constructs a basic UI from the parameters defined in the
    processor. However, this header is kept for completeness and future
    extension. If you wish to create a custom interface (e.g. sliders for
    retune speed, key selection, etc.), derive from juce::AudioProcessorEditor
    and design your layout here.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

//==============================================================================
/**
*/
class AutotuneAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AutotuneAudioProcessorEditor (AutotuneAudioProcessor& p)  : AudioProcessorEditor (&p), processor (p)
    {
        // You can add your own controls here. For example:
        // addAndMakeVisible (mixSlider);
        // mixSlider.setSliderStyle (juce::Slider::LinearVertical);
        // mixSlider.setRange (0.0, 1.0);
        // mixSlider.setValue (0.5);
        // mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        // mixAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (processor.parameters, AutotuneAudioProcessor::paramMix, mixSlider));

        setSize (400, 300);
    }
    ~AutotuneAudioProcessorEditor() override {}

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        g.setColour (juce::Colours::white);
        g.setFont (15.0f);
        g.drawFittedText ("Autotune Plugin", getLocalBounds(), juce::Justification::centred, 1);
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child components
        // that your editor contains.
    }

private:
    AutotuneAudioProcessor& processor;
    // Example slider parameter attachment (commented out for generic editor)
    // juce::Slider mixSlider;
    // std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutotuneAudioProcessorEditor)
};
