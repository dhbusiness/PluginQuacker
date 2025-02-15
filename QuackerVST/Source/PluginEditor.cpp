/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "LFOVisualizer.h"

juce::Image QuackerVSTAudioProcessorEditor::backgroundImage;
bool QuackerVSTAudioProcessorEditor::backgroundGenerated = false;

//==============================================================================
QuackerVSTAudioProcessorEditor::QuackerVSTAudioProcessorEditor (QuackerVSTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    // Generate background only if it hasn't been generated yet
    if (!backgroundGenerated)
    {
        generateBackgroundPattern(800, 600);
        backgroundGenerated = true;
    }
    
    startTimerHz(100); //Starting a timer which updates the GUI
    
    //Adding and init LFO rate and depth control params
    // Setup controls
    lfoRateSlider.setSliderStyle(juce::Slider::Rotary);
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    lfoRateSlider.setRange(0.01, 20.0, 0.01);
    addAndMakeVisible(lfoRateSlider);

    lfoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    lfoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    lfoDepthSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(lfoDepthSlider);


    
    waveformSelector.getComboBox().addItem("Sine", 1);
    waveformSelector.getComboBox().addItem("Square", 2);
    waveformSelector.getComboBox().addItem("Triangle", 3);
    waveformSelector.getComboBox().addItem("Sawtooth Up",4);
    waveformSelector.getComboBox().addItem("Sawtooth Down", 5);
    waveformSelector.getComboBox().addItem("Soft Square", 6);
    waveformSelector.getComboBox().addItem("Fender Style", 7);
    waveformSelector.getComboBox().addItem("Wurlitzer Style", 8);
    addAndMakeVisible(waveformSelector);


    //lfoSyncButton.setButtonText("Sync to BPM");
    addAndMakeVisible(lfoSyncButton);


    
    divisionSelector.getComboBox().addItem("1/1", 1);
    divisionSelector.getComboBox().addItem("1/2", 2);
    divisionSelector.getComboBox().addItem("1/4", 3);
    divisionSelector.getComboBox().addItem("1/8", 4);
    divisionSelector.getComboBox().addItem("1/16", 5);
    divisionSelector.getComboBox().addItem("1/32", 6);
    addAndMakeVisible(divisionSelector);
    
    // In your constructor
    waveformSelector.getComboBox().setJustificationType(juce::Justification::centred);
    divisionSelector.getComboBox().setJustificationType(juce::Justification::centred);

    
    // Style the ComboBoxes
    juce::Colour textColor = juce::Colour(232, 193, 185);  // Light rose gold
    auto setupComboBox = [textColor](juce::ComboBox& box) {
        box.setColour(juce::ComboBox::textColourId, textColor);
        box.setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
        box.setColour(juce::ComboBox::outlineColourId, juce::Colour(171, 136, 132));  // Darker rose gold
        box.setColour(juce::ComboBox::arrowColourId, textColor);
    };

    setupComboBox(lfoWaveformBox);
    setupComboBox(lfoNoteDivisionBox);

    lfoPhaseOffsetSlider.setSliderStyle(juce::Slider::Rotary);
    lfoPhaseOffsetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    lfoPhaseOffsetSlider.setRange(-180.0, 180.0, 1.0);
    lfoPhaseOffsetSlider.setTextValueSuffix(" °");
    addAndMakeVisible(lfoPhaseOffsetSlider);
    
    mixSlider.setSliderStyle(juce::Slider::Rotary);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    mixSlider.setRange(0.0f, 1.0f, 0.01f);
    addAndMakeVisible(mixSlider);
    
    // Labels and information stylising
    // Remove textbox borders
    lfoRateSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    lfoDepthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    lfoPhaseOffsetSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mixSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    // Set text colors to match our theme

    lfoRateSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    lfoDepthSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    lfoPhaseOffsetSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    mixSlider.setColour(juce::Slider::textBoxTextColourId, textColor);

    //Bypass
    bypassButton.setLookAndFeel(&customToggleLookAndFeel);
    addAndMakeVisible(bypassButton);
    
    // Set button texts (will appear above switches)
    lfoSyncButton.setButtonText("SYNC");
    bypassButton.setButtonText("BYPASS");
    
    // Create bypass attachment (you'll need to add a bypass parameter to your APVTS first)
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "bypass", bypassButton);

    
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
    
    
    lfoWaveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfoWaveform", waveformSelector.getComboBox());
    lfoNoteDivisionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfoNoteDivision", divisionSelector.getComboBox());
    //
    addAndMakeVisible(lfoVisualizer);
    
    //LookandFeel
    lfoRateSlider.setLookAndFeel(&customDialLookAndFeel);
    lfoDepthSlider.setLookAndFeel(&customDialLookAndFeel);
    lfoPhaseOffsetSlider.setLookAndFeel(&customDialLookAndFeel);
    mixSlider.setLookAndFeel(&customDialLookAndFeel);
    
    lfoSyncButton.setLookAndFeel(&customToggleLookAndFeel);
    
    lfoWaveformBox.setLookAndFeel(&customComboBoxLookAndFeel);
    lfoNoteDivisionBox.setLookAndFeel(&customComboBoxLookAndFeel);
    
    waveformSelector.getComboBox().setLookAndFeel(&customComboBoxLookAndFeel);
    divisionSelector.getComboBox().setLookAndFeel(&customComboBoxLookAndFeel);
    // Add mouse listeners for the arrows
    lfoWaveformBox.addMouseListener(this, false);
    lfoNoteDivisionBox.addMouseListener(this, false);

    
}

QuackerVSTAudioProcessorEditor::~QuackerVSTAudioProcessorEditor()
{
    stopTimer(); //Stopping the timer in deconstructor
    // Clean up the look and feel
    lfoRateSlider.setLookAndFeel(nullptr);
    lfoDepthSlider.setLookAndFeel(nullptr);
    lfoPhaseOffsetSlider.setLookAndFeel(nullptr);
    mixSlider.setLookAndFeel(nullptr);
    
    lfoSyncButton.setLookAndFeel(nullptr);
    
    bypassButton.setLookAndFeel(nullptr);
    
    lfoWaveformBox.setLookAndFeel(nullptr);
    lfoNoteDivisionBox.setLookAndFeel(nullptr);
}

void QuackerVSTAudioProcessorEditor::timerCallback()
{
    // Get bypass state
    auto* bypassParam = audioProcessor.apvts.getRawParameterValue("bypass");
    bool isBypassed = bypassParam->load();

    // Only update visualizer if not bypassed
    if (!isBypassed)
    {
        bool isActive = audioProcessor.isPlaying() && audioProcessor.hasAudioInput();
        lfoVisualizer.setActive(isActive, audioProcessor.isLfoWaitingForReset());
        
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
    }
    else
    {
        // When bypassed, stop the LFO visualization
        lfoVisualizer.setActive(false, false);
    }
    
    repaint();
}
    
//==============================================================================
void QuackerVSTAudioProcessorEditor::generateBackgroundPattern(int width, int height)
{
    backgroundImage = juce::Image(juce::Image::ARGB, width, height, true);
    juce::Graphics g(backgroundImage);
    
    auto bounds = juce::Rectangle<float>(0, 0, width, height);
    
    // Enhanced color palette with metallic tones
    juce::Colour darkPlum(61, 21, 46);
    juce::Colour midPlum(72, 28, 55);
    juce::Colour lightPlum(89, 34, 68);
    juce::Colour peachPink = juce::Colour(255, 201, 190).withMultipliedBrightness(0.6f);
    juce::Colour roseGold = juce::Colour(232, 193, 185).withMultipliedBrightness(0.6f);
    juce::Colour warmPlum = juce::Colour(198, 109, 139).withMultipliedBrightness(0.7f);
    
    // Metallic sheen colors - adjusted for more uniformity
    juce::Colour metalHighlight = juce::Colour(255, 255, 255).withAlpha(0.03f);
    juce::Colour metalShadow = juce::Colours::black.withAlpha(0.05f);

    // Base gradient with more uniform metallic effect
    juce::ColourGradient baseGradient(
        darkPlum.brighter(0.05f),  // Reduced brightness difference
        bounds.getCentreX(), bounds.getCentreY(),  // Changed to center-based gradient
        midPlum.darker(0.05f),     // Reduced darkness difference
        bounds.getRight(), bounds.getBottom(),
        true);
    
    baseGradient.addColour(0.3, midPlum);
    baseGradient.addColour(0.7, lightPlum);
    
    g.setGradientFill(baseGradient);
    g.fillAll();

    // More uniform metallic sheen
    juce::ColourGradient sheenGradient(
        metalHighlight,
        bounds.getCentreX(), bounds.getCentreY(),  // Center-based gradient
        metalShadow,
        bounds.getRight(), bounds.getBottom(),
        true);  // Changed to radial gradient
    
    g.setGradientFill(sheenGradient);
    g.fillAll();

    // Enhanced metal flakes
    juce::Random random;
    for (int i = 0; i < 15000; ++i) // Increased number of flakes
    {
        float x = random.nextFloat() * width;
        float y = random.nextFloat() * height;
        float size = random.nextFloat() * 1.8f; // Slightly larger flakes
        float alpha = random.nextFloat() * 0.08f; // Increased opacity

        // Randomize between highlight and shadow for flakes
        if (random.nextBool())
        {
            g.setColour(juce::Colours::white.withAlpha(alpha));
        }
        else
        {
            g.setColour(metalShadow.withAlpha(alpha * 0.8f));
        }
        
        g.fillEllipse(x, y, size, size);
    }

    // Add your existing Perlin noise patterns with adjusted alpha
    const float scale = 0.007f;
    const float fixedSeed = 42.0f;
    
    struct LayerConfig {
        float scale;
        float alpha;
        float amplitude;
        juce::Colour color;
        float offset;
    };

    // Adjust layers for metallic effect
    std::array<LayerConfig, 5> layers = {{
        { 0.003f, 0.08f, 8.0f, warmPlum, 0.0f },
        { 0.005f, 0.06f, 6.0f, roseGold, 50.0f },
        { 0.008f, 0.04f, 5.0f, peachPink, 100.0f },
        { 0.002f, 0.08f, 10.0f, lightPlum, 150.0f },
        { 0.004f, 0.03f, 6.0f, warmPlum, 200.0f }
    }};

    // Generate each texture layer
    for (const auto& layer : layers)
    {
        juce::Path swirlyPath;
        
        for (float y = 0; y < bounds.getHeight(); y += 4.0f)
        {
            swirlyPath.startNewSubPath(0, y);
            
            for (float x = 0; x < bounds.getWidth(); x += 4.0f)
            {
                float noise1 = PerlinNoise::noise(x * layer.scale, y * layer.scale, fixedSeed + layer.offset);
                float noise2 = PerlinNoise::noise(x * layer.scale * 1.7f, y * layer.scale * 1.7f, fixedSeed + layer.offset + 10.0f);
                float noise3 = PerlinNoise::noise(y * layer.scale * 0.5f, x * layer.scale * 0.5f, fixedSeed + layer.offset + 20.0f);
                
                float combinedNoise = (noise1 + noise2 * 0.5f + noise3 * 0.25f) * juce::MathConstants<float>::pi * 4;
                
                float offsetX = std::sin(combinedNoise) * layer.amplitude;
                float offsetY = std::cos(combinedNoise) * layer.amplitude;
                
                swirlyPath.lineTo(x + offsetX, y + offsetY);
            }
        }
        
        g.setColour(layer.color.withAlpha(layer.alpha));
        g.strokePath(swirlyPath, juce::PathStrokeType(1.5f));
    }

    // Final metallic sheen overlay
    // Final metallic sheen overlay - more subtle and uniform
    juce::ColourGradient finalSheen(
        metalHighlight.withAlpha(0.01f),
        bounds.getCentreX(), bounds.getCentreY(),
        metalShadow.withAlpha(0.01f),
        bounds.getRight(), bounds.getBottom(),
        true);  // Changed to radial gradient
    
    g.setGradientFill(finalSheen);
    g.fillAll();
}

void QuackerVSTAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Draw cached background
    g.drawImageAt(backgroundImage, 0, 0);
    
    // Draw controls on top
    drawControls(g);
}

void QuackerVSTAudioProcessorEditor::drawControls(juce::Graphics& g)
{
    // Your existing control drawing code here
    g.setColour(juce::Colour(232, 193, 185));  // Light rose gold
    g.setFont(16.0f);

    const int dialSize = 150;
    const int spacing = 20;
    const int totalWidth = (dialSize * 4) + (spacing * 3);
    const int startX = (getWidth() - totalWidth) / 2;
    const int labelY = 210 + dialSize + 5;

    g.drawText("RATE",
               juce::Rectangle<int>(startX, labelY, dialSize, 20),
               juce::Justification::centred);
               
    g.drawText("DEPTH",
               juce::Rectangle<int>(startX + dialSize + spacing, labelY, dialSize, 20),
               juce::Justification::centred);
               
    g.drawText("WAVE OFFSET",
               juce::Rectangle<int>(startX + (dialSize + spacing) * 2, labelY, dialSize, 20),
               juce::Justification::centred);
               
    g.drawText("MIX",
               juce::Rectangle<int>(startX + (dialSize + spacing) * 3, labelY, dialSize, 20),
               juce::Justification::centred);
}

void QuackerVSTAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    if (auto* box = dynamic_cast<juce::ComboBox*>(event.eventComponent))
    {
        auto bounds = box->getLocalBounds();
        auto arrowWidth = bounds.getHeight();
        
        // Check if click is in left arrow area
        if (event.x < arrowWidth)
        {
            int currentIndex = box->getSelectedItemIndex();
            if (currentIndex > 0)
                box->setSelectedItemIndex(currentIndex - 1);
        }
        // Check if click is in right arrow area
        else if (event.x > bounds.getWidth() - arrowWidth)
        {
            int currentIndex = box->getSelectedItemIndex();
            if (currentIndex < box->getNumItems() - 1)
                box->setSelectedItemIndex(currentIndex + 1);
        }
    }
}

void QuackerVSTAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve top section for visualizer
    auto visualizerBounds = bounds.removeFromTop(200);
    visualizerBounds.reduce(10, 10);
    lfoVisualizer.setBounds(visualizerBounds);
    
    // Calculate sizes for uniform dials
    const int dialSize = 150;
    const int labelHeight = 20;
    const int spacing = 20;
    
    // Calculate total width needed for dials
    const int totalWidth = (dialSize * 4) + (spacing * 3);
    const int startX = (getWidth() - totalWidth) / 2;
    const int startY = visualizerBounds.getBottom() + spacing;

    // Position dials
    lfoRateSlider.setBounds(startX, startY, dialSize, dialSize);
    lfoDepthSlider.setBounds(startX + dialSize + spacing, startY, dialSize, dialSize);
    lfoPhaseOffsetSlider.setBounds(startX + (dialSize + spacing) * 2, startY, dialSize, dialSize);
    mixSlider.setBounds(startX + (dialSize + spacing) * 3, startY, dialSize, dialSize);

    // ComboBox dimensions
    const int comboBoxWidth = 140;
    const int comboBoxHeight = 25;
    const int comboY = startY + dialSize + spacing + 25;

    // Center the combo boxes under their respective dials
    const int rateDialCenterX = startX + (dialSize / 2);
    const int mixDialCenterX = startX + (dialSize + spacing) * 3 + (dialSize / 2);
    
    // Position combo boxes centered under their dials
    divisionSelector.setBounds(rateDialCenterX - (comboBoxWidth / 2), comboY, comboBoxWidth, comboBoxHeight);
    waveformSelector.setBounds(mixDialCenterX - (comboBoxWidth / 2), comboY, comboBoxWidth, comboBoxHeight);

    // Position buttons below their respective combo boxes
    const int buttonWidth = 100;
    const int buttonHeight = 40;
    const int buttonsY = comboY + comboBoxHeight + spacing;

    // Center the buttons under their corresponding combo boxes
    lfoSyncButton.setBounds(rateDialCenterX - (buttonWidth / 2), buttonsY, buttonWidth, buttonHeight);
    bypassButton.setBounds(mixDialCenterX - (buttonWidth / 2), buttonsY, buttonWidth, buttonHeight);
}
