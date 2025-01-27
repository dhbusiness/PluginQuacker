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
    
    /*
    //added gain slider settings
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setRange(0.0, 2.0, 0.01);
    gainSlider.setValue(1.0);
    gainSlider.onValueChange = [this] { audioProcessor.gainParameter->setValueNotifyingHost(gainSlider.getValue()); };
    addAndMakeVisible(&gainSlider);
    */
    
    //Adding and config ADSR sliders
    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setRange(0.01, 2.0, 0.01);
    attackSlider.setValue(audioProcessor.attackParam->get());
    attackSlider.onValueChange = [this] {audioProcessor.attackParam->setValueNotifyingHost(attackSlider.getValue()); };
    addAndMakeVisible(attackSlider);
    
    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setRange(0.01, 2.0, 0.01);
    decaySlider.setValue(audioProcessor.decayParam->get());
    decaySlider.onValueChange = [this] { audioProcessor.decayParam->setValueNotifyingHost(decaySlider.getValue()); };
    addAndMakeVisible(decaySlider);
    

    sustainSlider.setSliderStyle(juce::Slider::LinearVertical);
    sustainSlider.setRange(0.0, 1.0, 0.01);
    sustainSlider.setValue(audioProcessor.sustainParam->get());
    sustainSlider.onValueChange = [this] { audioProcessor.sustainParam->setValueNotifyingHost(sustainSlider.getValue()); };
    addAndMakeVisible(sustainSlider);

    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setRange(0.01, 2.0, 0.01);
    releaseSlider.setValue(audioProcessor.releaseParam->get());
    releaseSlider.onValueChange = [this] { audioProcessor.releaseParam->setValueNotifyingHost(releaseSlider.getValue()); };
    addAndMakeVisible(releaseSlider);
    
    // Configure toggle button for enabling/disabling ducking
    duckingToggle.setButtonText("Enable Ducking");
    duckingToggle.setToggleState(audioProcessor.duckingEnabled->get(), juce::dontSendNotification);
    duckingToggle.onClick = [this] { audioProcessor.duckingEnabled->setValueNotifyingHost(duckingToggle.getToggleState()); };
    addAndMakeVisible(duckingToggle);

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
    
    auto bpm = audioProcessor.getCurrentBPM();
    auto QuarterNoteMS = 60000/bpm;
    auto WholeNoteMS = QuarterNoteMS * 4;
    auto HalfNoteMS = WholeNoteMS * 0.5;
    auto EightNoteMS = WholeNoteMS * 0.125;
    auto SixteenthNotesMS = WholeNoteMS * 0.0625;
    auto ThirtyTwoNotesMS = WholeNoteMS * 0.03125;
    
    float SelectedTimeMS = QuarterNoteMS;
    float ResetTime = SelectedTimeMS / 2.0;
    
    
    
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
    attackSlider.setBounds(bounds.removeFromLeft(80));
    decaySlider.setBounds(bounds.removeFromLeft(80));
    sustainSlider.setBounds(bounds.removeFromLeft(80));
    releaseSlider.setBounds(bounds.removeFromLeft(80));
    duckingToggle.setBounds(bounds.removeFromTop(40));
}
