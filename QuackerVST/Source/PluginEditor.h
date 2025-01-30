/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class QuackerVSTAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
    //added juce::Timer inheritance
{
public:
    QuackerVSTAudioProcessorEditor (QuackerVSTAudioProcessor&);
    ~QuackerVSTAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override; //Override this function from juce::Timer to modify in cpp

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    QuackerVSTAudioProcessor& audioProcessor;
    //juce::Slider gainSlider; //Added slider for gain
    juce::Slider lfoRateSlider, lfoDepthSlider;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessorEditor)
};
