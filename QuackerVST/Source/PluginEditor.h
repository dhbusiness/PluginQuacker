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
#include "PresetComponent.h"

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
    void timerCallback() override;
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
    
    // Add PresetComponent
    PresetComponent presetComponent; 
    
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
    
    // Waveshaping controls
     juce::Slider waveshapeRateSlider, waveshapeDepthSlider;
     ArrowNavigationComboBox waveshapeWaveformSelector;
     juce::ToggleButton waveshapeEnableButton;

     // Attachments for waveshaping controls
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> waveshapeRateAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> waveshapeDepthAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveshapeWaveformAttachment;
     std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> waveshapeEnableAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessorEditor)
};
