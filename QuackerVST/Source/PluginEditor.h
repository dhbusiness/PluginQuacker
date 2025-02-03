/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LFOVisualizer.h"
#include "CustomDialLookAndFeel.h"
#include "CustomToggle.h"
#include "PerlinNoise.h"
#include "CustomComboBox.h"
#include "ArrowNavigationComboBox.h"


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
    void mouseDown(const juce::MouseEvent& event) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    QuackerVSTAudioProcessor& audioProcessor;
    
    LFOVisualizer lfoVisualizer;
    
    juce::Slider lfoRateSlider, lfoDepthSlider; //Defining sliders for LFO control
    juce::ComboBox lfoWaveformBox;
    
    juce::ToggleButton lfoSyncButton;
    juce::ComboBox lfoNoteDivisionBox;
    
    juce::Slider lfoPhaseOffsetSlider;
    
    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoWaveformAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoSyncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoNoteDivisionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoPhaseOffsetAttachment;
    
    //Look and feel
    CustomDial customDialLookAndFeel;
    CustomToggle customToggleLookAndFeel;
    CustomComboBox customComboBoxLookAndFeel;
    ArrowNavigationComboBox waveformSelector;
    ArrowNavigationComboBox divisionSelector;
    
    //Bypass
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    
    float backgroundPhase = 0.0f;
    void drawControls(juce::Graphics& g);
    static juce::Image backgroundImage;  // Make it static
    static bool backgroundGenerated;     // Track if we've generated it
    static void generateBackgroundPattern(int width, int height); // Static generator
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessorEditor)
};
