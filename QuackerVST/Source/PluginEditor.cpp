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
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    lfoRateSlider.setRange(0.01, 2.0, 0.01);
    addAndMakeVisible(lfoRateSlider);

    lfoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    lfoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
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
    lfoPhaseOffsetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
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
    
    //LookandFeel
    lfoRateSlider.setLookAndFeel(&customDialLookAndFeel);
    lfoDepthSlider.setLookAndFeel(&customDialLookAndFeel);
    lfoPhaseOffsetSlider.setLookAndFeel(&customDialLookAndFeel);
    mixSlider.setLookAndFeel(&customDialLookAndFeel);

    
}

QuackerVSTAudioProcessorEditor::~QuackerVSTAudioProcessorEditor()
{
    stopTimer(); //Stopping the timer in deconstructor
    // Clean up the look and feel
    lfoRateSlider.setLookAndFeel(nullptr);
    lfoDepthSlider.setLookAndFeel(nullptr);
    lfoPhaseOffsetSlider.setLookAndFeel(nullptr);
    mixSlider.setLookAndFeel(nullptr);
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
    auto bounds = getLocalBounds().toFloat();
    
    // Base layer - deep plum core
    juce::ColourGradient baseGradient(
        juce::Colour(61, 21, 46),  // Darker plum center
        bounds.getCentreX(), bounds.getCentreY(),
        juce::Colour(89, 34, 68),  // Natural plum edge
        0, 0,
        true);  // Radial gradient
    
    // Add subtle color variations for depth
    baseGradient.addColour(0.3, juce::Colour(72, 28, 55));
    baseGradient.addColour(0.7, juce::Colour(95, 41, 73));
    
    g.setGradientFill(baseGradient);
    g.fillAll();

    // Second layer - peachy highlights
    juce::ColourGradient peachGradient(
        juce::Colour(255, 201, 190).withAlpha(0.2f),  // Peachy pink
        bounds.getRight(), 0,
        juce::Colour(236, 174, 172).withAlpha(0.0f),  // Transparent peach
        bounds.getCentreX(), bounds.getCentreY(),
        false);  // Linear gradient
        
    g.setGradientFill(peachGradient);
    g.fillAll();

    // Bloom effect - top left
    juce::ColourGradient bloomGradient(
        juce::Colour(198, 109, 139).withAlpha(0.15f),  // Light plum
        bounds.getX(), bounds.getY(),
        juce::Colour(198, 109, 139).withAlpha(0.0f),
        bounds.getX() + 300, bounds.getY() + 300,
        true);  // Radial
        
    g.setGradientFill(bloomGradient);
    g.fillAll();

    // Add subtle velvety texture
    for (int i = 0; i < 200; ++i) {
        float x = juce::Random::getSystemRandom().nextFloat() * bounds.getWidth();
        float y = juce::Random::getSystemRandom().nextFloat() * bounds.getHeight();
        float size = juce::Random::getSystemRandom().nextFloat() * 2.0f + 0.5f;
        
        g.setColour(juce::Colour(255, 255, 255).withAlpha(0.01f));
        g.fillEllipse(x, y, size, size);
    }

    // Add darker dusting effect
    for (int i = 0; i < 150; ++i) {
        float x = juce::Random::getSystemRandom().nextFloat() * bounds.getWidth();
        float y = juce::Random::getSystemRandom().nextFloat() * bounds.getHeight();
        float size = juce::Random::getSystemRandom().nextFloat() * 1.5f + 0.5f;
        
        g.setColour(juce::Colour(0, 0, 0).withAlpha(0.02f));
        g.fillEllipse(x, y, size, size);
    }

    // Add highlights
    juce::Path highlightPath;
    float highlightWidth = bounds.getWidth() / 8;
    float highlightHeight = bounds.getHeight() / 3;
    highlightPath.addEllipse(50, 50, highlightWidth, highlightHeight);
    
    juce::ColourGradient highlightGradient(
        juce::Colour(255, 255, 255).withAlpha(0.05f),
        50, 50,
        juce::Colour(255, 255, 255).withAlpha(0.0f),
        50 + highlightWidth, 50 + highlightHeight,
        true);
        
    g.setGradientFill(highlightGradient);
    g.fillPath(highlightPath);

    // Add a subtle waxy sheen
    juce::ColourGradient sheenGradient(
        juce::Colour(255, 255, 255).withAlpha(0.03f),
        bounds.getCentreX() - 100, bounds.getCentreY() - 100,
        juce::Colour(255, 255, 255).withAlpha(0.0f),
        bounds.getCentreX() + 100, bounds.getCentreY() + 100,
        true);
        
    g.setGradientFill(sheenGradient);
    g.fillAll();
}

void QuackerVSTAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve top section for visualizer
    auto visualizerBounds = bounds.removeFromTop(200);
    visualizerBounds.reduce(10, 10);
    lfoVisualizer.setBounds(visualizerBounds);
    
    // Calculate sizes for uniform dials
    const int dialSize = 150;  // Using the size of the mix dial
    const int textBoxHeight = 20;
    const int spacing = 20;
    
    // Calculate total width needed
    const int totalWidth = (dialSize * 4) + (spacing * 3);
    // Calculate starting X to center the dials
    const int startX = (getWidth() - totalWidth) / 2;
    const int startY = visualizerBounds.getBottom() + spacing;

    // Position dials with equal size and spacing
    lfoRateSlider.setBounds(startX, startY, dialSize, dialSize);
    lfoDepthSlider.setBounds(startX + dialSize + spacing, startY, dialSize, dialSize);
    lfoPhaseOffsetSlider.setBounds(startX + (dialSize + spacing) * 2, startY, dialSize, dialSize);
    mixSlider.setBounds(startX + (dialSize + spacing) * 3, startY, dialSize, dialSize);
    
    // Position comboboxes and buttons below dials
    const int controlY = startY + dialSize + spacing;
    const int comboBoxWidth = dialSize;
    const int comboBoxHeight = 30;
    
    lfoWaveformBox.setBounds(startX, controlY, comboBoxWidth, comboBoxHeight);
    lfoSyncButton.setBounds(startX + dialSize + spacing, controlY, comboBoxWidth, comboBoxHeight);
    lfoNoteDivisionBox.setBounds(startX + (dialSize + spacing) * 2, controlY, comboBoxWidth, comboBoxHeight);
}
