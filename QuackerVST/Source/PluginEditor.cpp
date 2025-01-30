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
    lfoRateSlider.setSliderStyle(juce::Slider::Rotary);
    lfoRateSlider.setRange(0.01, 2.0, 0.01);
    lfoRateSlider.setValue(audioProcessor.lfoRateParam->get());
    lfoRateSlider.onValueChange = [this] {
        audioProcessor.lfoRateParam->setValueNotifyingHost(lfoRateSlider.getValue());
    };
    addAndMakeVisible(lfoRateSlider);

    lfoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    lfoDepthSlider.setRange(0.0, 1.0, 0.01);
    lfoDepthSlider.setValue(audioProcessor.lfoDepthParam->get());
    lfoDepthSlider.onValueChange = [this] {
        audioProcessor.lfoDepthParam->setValueNotifyingHost(lfoDepthSlider.getValue());
    };
    addAndMakeVisible(lfoDepthSlider);
    
    //Adding and init LFO waveform choice
    lfoWaveformBox.addItem("Sine", 1);
    lfoWaveformBox.addItem("Square", 2);
    lfoWaveformBox.addItem("Triangle", 3);
    lfoWaveformBox.setSelectedId(audioProcessor.lfoWaveformParam->getIndex() + 1, juce::dontSendNotification);
    lfoWaveformBox.onChange = [this] {
        audioProcessor.lfoWaveformParam->setValueNotifyingHost(lfoWaveformBox.getSelectedId() - 1);
    };
    addAndMakeVisible(lfoWaveformBox);
    
    //Adding LFO sync toggle
    lfoSyncButton.setButtonText("Sync to BPM");
    lfoSyncButton.setToggleState(audioProcessor.lfoSyncParam->get(), juce::dontSendNotification);
    lfoSyncButton.onClick = [this] {
        audioProcessor.lfoSyncParam->setValueNotifyingHost(lfoSyncButton.getToggleState());
    };
    addAndMakeVisible(lfoSyncButton);
    
    //Adding note division selection
    lfoNoteDivisionBox.addItem("Whole", 1);
    lfoNoteDivisionBox.addItem("Half", 2);
    lfoNoteDivisionBox.addItem("Quarter", 3);
    lfoNoteDivisionBox.addItem("Eighth", 4);
    lfoNoteDivisionBox.addItem("Sixteenth", 5);
    lfoNoteDivisionBox.setSelectedId(audioProcessor.lfoNoteDivisionParam->getIndex() + 1, juce::dontSendNotification);
    lfoNoteDivisionBox.onChange = [this] {
        audioProcessor.lfoNoteDivisionParam->setValueNotifyingHost(lfoNoteDivisionBox.getSelectedId() - 1);
    };
    addAndMakeVisible(lfoNoteDivisionBox);
    
    //Adding phase offset (Feel) controls
    lfoPhaseOffsetSlider.setSliderStyle(juce::Slider::Rotary);
    lfoPhaseOffsetSlider.setRange(-180.0, 180.0, 1.0);
    lfoPhaseOffsetSlider.setValue(audioProcessor.lfoPhaseOffsetParam->get());
    lfoPhaseOffsetSlider.setTextValueSuffix(" °");
    lfoPhaseOffsetSlider.onValueChange = [this] {
        audioProcessor.lfoPhaseOffsetParam->setValueNotifyingHost(lfoPhaseOffsetSlider.getValue());
    };
    addAndMakeVisible(lfoPhaseOffsetSlider);
    
    
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
    lfoVisualizer.setWaveform(audioProcessor.lfoWaveformParam->getIndex());
    lfoVisualizer.setDepth(audioProcessor.lfoDepthParam->get());
    lfoVisualizer.setPhaseOffset(audioProcessor.lfoPhaseOffsetParam->get());
    
    // Update rate and sync settings
    if (audioProcessor.lfoSyncParam->get())
    {
        lfoVisualizer.setTempoSync(true,
                                  audioProcessor.getCurrentBPM(),
                                  audioProcessor.lfoNoteDivisionParam->getIndex());
    }
    else
    {
        lfoVisualizer.setRate(audioProcessor.lfoRateParam->get());
    }
    
    repaint();  // Repaint the editor periodically
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
    visualizerBounds.reduce(10, 10); // Add some padding
    lfoVisualizer.setBounds(visualizerBounds);
    
    // Existing controls
    lfoRateSlider.setBounds(10, visualizerBounds.getBottom() + 10, 150, 150);
    lfoDepthSlider.setBounds(170, visualizerBounds.getBottom() + 10, 150, 150);
    lfoWaveformBox.setBounds(10, lfoRateSlider.getBottom() + 10, 150, 30);
    lfoSyncButton.setBounds(10, lfoWaveformBox.getBottom() + 10, 150, 30);
    lfoNoteDivisionBox.setBounds(170, lfoWaveformBox.getBottom() + 10, 150, 30);
    lfoPhaseOffsetSlider.setBounds(330, visualizerBounds.getBottom() + 10, 150, 150);

    
}
