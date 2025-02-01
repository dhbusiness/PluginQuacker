/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "LFOVisualizer.h"

//==============================================================================
QuackerVSTAudioProcessorEditor::QuackerVSTAudioProcessorEditor (QuackerVSTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    startTimerHz(100); //Starting a timer which updates the GUI
    
    //Adding and init LFO rate and depth control params
    // Setup controls
    lfoRateSlider.setSliderStyle(juce::Slider::Rotary);
    lfoRateSlider.setRange(0.01, 2.0, 0.01);
    addAndMakeVisible(lfoRateSlider);

    lfoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    lfoDepthSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(lfoDepthSlider);

    lfoWaveformBox.addItem("Sine", 1);
    lfoWaveformBox.addItem("Square", 2);
    lfoWaveformBox.addItem("Triangle", 3);
    lfoWaveformBox.addItem("Sawtooth Up",4);
    lfoWaveformBox.addItem("Sawtooth Down", 5);
    lfoWaveformBox.addItem("Soft Square", 6);
    lfoWaveformBox.addItem("Fender Style", 7);
    lfoWaveformBox.addItem("Wurlitzer Style", 8);
    addAndMakeVisible(lfoWaveformBox);

    lfoSyncButton.setButtonText("Sync to BPM");
    addAndMakeVisible(lfoSyncButton);

    lfoNoteDivisionBox.addItem("Whole", 1);
    lfoNoteDivisionBox.addItem("Half", 2);
    lfoNoteDivisionBox.addItem("Quarter", 3);
    lfoNoteDivisionBox.addItem("Eighth", 4);
    lfoNoteDivisionBox.addItem("Sixteenth", 5);
    addAndMakeVisible(lfoNoteDivisionBox);

    lfoPhaseOffsetSlider.setSliderStyle(juce::Slider::Rotary);
    lfoPhaseOffsetSlider.setRange(-180.0, 180.0, 1.0);
    lfoPhaseOffsetSlider.setTextValueSuffix(" °");
    addAndMakeVisible(lfoPhaseOffsetSlider);
    
    mixSlider.setSliderStyle(juce::Slider::Rotary);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    mixSlider.setRange(0.0f, 1.0f, 0.01f);
    addAndMakeVisible(mixSlider);


    
    // Create attachments
    lfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "lfoRate", lfoRateSlider);
    lfoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "lfoDepth", lfoDepthSlider);
    lfoWaveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfoWaveform", lfoWaveformBox);
    lfoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "lfoSync", lfoSyncButton);
    lfoNoteDivisionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfoNoteDivision", lfoNoteDivisionBox);
    lfoPhaseOffsetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "lfoPhaseOffset", lfoPhaseOffsetSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "mix", mixSlider);
    
    //
    addAndMakeVisible(lfoVisualizer);

    
}

QuackerVSTAudioProcessorEditor::~QuackerVSTAudioProcessorEditor()
{
    stopTimer(); //Stopping the timer in deconstructor
}

void QuackerVSTAudioProcessorEditor::timerCallback()
{
    // Update visualizer with current parameter values
    // Get parameters from APVTS
    auto waveformParam = audioProcessor.apvts.getRawParameterValue("lfoWaveform");
    auto depthParam = audioProcessor.apvts.getRawParameterValue("lfoDepth");
    auto phaseOffsetParam = audioProcessor.apvts.getRawParameterValue("lfoPhaseOffset");
    auto syncParam = audioProcessor.apvts.getRawParameterValue("lfoSync");
    auto rateParam = audioProcessor.apvts.getRawParameterValue("lfoRate");
    auto divisionParam = audioProcessor.apvts.getRawParameterValue("lfoNoteDivision");

    // Update visualizer with current parameter values
    lfoVisualizer.setWaveform(static_cast<int>(waveformParam->load()));
    lfoVisualizer.setDepth(depthParam->load());
    lfoVisualizer.setPhaseOffset(phaseOffsetParam->load());
    
    // Update rate and sync settings
    if (syncParam->load() > 0.5f)
    {
        lfoVisualizer.setTempoSync(true,
                                  audioProcessor.getCurrentBPM(),
                                  static_cast<int>(divisionParam->load()));
    }
    else
    {
        lfoVisualizer.setRate(rateParam->load());
    }
    
    repaint();
    }
    
//==============================================================================
void QuackerVSTAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    
    
    /*
    g.drawFittedText ("BPM: " + juce::String(bpm), getLocalBounds(), juce::Justification::centred, 1);
    g.drawFittedText ("WholeNoteMS: " + juce::String(WholeNoteMS), getLocalBounds(), juce::Justification::centred - 10, 1);
    g.drawFittedText ("HalfNoteMS: " + juce::String(HalfNoteMS), getLocalBounds(), juce::Justification::centred + 15, 1);
    g.drawFittedText ("QuarterNoteMS: " + juce::String(QuarterNoteMS), getLocalBounds(), juce::Justification::centred + 20, 1);
    g.drawFittedText ("EightNoteMS: " + juce::String(EightNoteMS), getLocalBounds(), juce::Justification::centred + 25, 1);
    g.drawFittedText ("SixteenthNotesMS: " + juce::String(SixteenthNotesMS), getLocalBounds(), juce::Justification::centred + 30, 1);
    g.drawFittedText ("ThirtyTwoNotesMS: " + juce::String(ThirtyTwoNotesMS), getLocalBounds(), juce::Justification::centred + 35, 1);
     */
    
}

void QuackerVSTAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    
    // Reserve top section for visualizer
    auto visualizerBounds = bounds.removeFromTop(200);
    visualizerBounds.reduce(10, 10);
    lfoVisualizer.setBounds(visualizerBounds);
    
    // Position existing controls
    lfoRateSlider.setBounds(10, visualizerBounds.getBottom() + 10, 150, 150);
    lfoDepthSlider.setBounds(170, visualizerBounds.getBottom() + 10, 150, 150);
    lfoPhaseOffsetSlider.setBounds(330, visualizerBounds.getBottom() + 10, 150, 150);
    
    // Add mix control
    mixSlider.setBounds(490, visualizerBounds.getBottom() + 10, 150, 150);
    
    // Bottom row controls
    lfoWaveformBox.setBounds(10, lfoRateSlider.getBottom() + 10, 150, 30);
    lfoSyncButton.setBounds(10, lfoWaveformBox.getBottom() + 10, 150, 30);
    lfoNoteDivisionBox.setBounds(170, lfoWaveformBox.getBottom() + 10, 150, 30);
    
}
