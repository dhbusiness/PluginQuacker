/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
QuackerVSTAudioProcessorEditor::QuackerVSTAudioProcessorEditor (QuackerVSTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    startTimerHz(10); //Starting a timer which updates the GUI
    
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
    

    
}

QuackerVSTAudioProcessorEditor::~QuackerVSTAudioProcessorEditor()
{
    stopTimer(); //Stopping the timer in deconstructor
}

void QuackerVSTAudioProcessorEditor::timerCallback()
{
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
    //gainSlider.setBounds(10, 10, getWidth() - 20, 30);
    auto bounds = getLocalBounds();
    
    lfoRateSlider.setBounds(10, 10, 150, 150);
    lfoDepthSlider.setBounds(170, 10, 150, 150);

}
