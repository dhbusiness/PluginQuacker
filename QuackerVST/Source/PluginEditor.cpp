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
    setSize (800, 700);

    // Generate background only if it hasn't been generated yet
    if (!backgroundGenerated)
    {
        generateBackgroundPattern(800, 700);
        backgroundGenerated = true;
    }
    
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
    
    //
    // Setup modulation controls
    modRateSlider.setSliderStyle(juce::Slider::Rotary);
    modRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    modRateSlider.setRange(0.01f, 5.0f, 0.01f);
    modRateSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    modRateSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    modRateSlider.setLookAndFeel(&customDialLookAndFeel);
    addAndMakeVisible(modRateSlider);

    modDepthSlider.setSliderStyle(juce::Slider::Rotary);
    modDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    modDepthSlider.setRange(0.0f, 1.0f, 0.01f);
    modDepthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    modDepthSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    modDepthSlider.setLookAndFeel(&customDialLookAndFeel);
    addAndMakeVisible(modDepthSlider);

    // Setup modulation ComboBoxes
    modWaveformSelector.getComboBox().addItem("Sine", 1);
    modWaveformSelector.getComboBox().addItem("Triangle", 2);
    modWaveformSelector.getComboBox().addItem("Square", 3);
    modWaveformSelector.getComboBox().addItem("Saw", 4);
    modWaveformSelector.getComboBox().setJustificationType(juce::Justification::centred);
    modWaveformSelector.getComboBox().setLookAndFeel(&customComboBoxLookAndFeel);
    addAndMakeVisible(modWaveformSelector);

    modTargetSelector.getComboBox().addItem("Rate", 1);
    modTargetSelector.getComboBox().addItem("Depth", 2);
    modTargetSelector.getComboBox().addItem("Phase", 3);
    modTargetSelector.getComboBox().setJustificationType(juce::Justification::centred);
    modTargetSelector.getComboBox().setLookAndFeel(&customComboBoxLookAndFeel);
    //addAndMakeVisible(modTargetSelector);
    
    modEnableButton.setButtonText("MOD ON");
    modEnableButton.setLookAndFeel(&customToggleLookAndFeel);
    addAndMakeVisible(modEnableButton);

    modEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "modEnable", modEnableButton);

    // Create modulation parameter attachments
    modRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "modLfoRate", modRateSlider);
    modDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "modLfoDepth", modDepthSlider);
    modWaveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "modLfoWaveform", modWaveformSelector.getComboBox());
    modTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "modTarget", modTargetSelector.getComboBox());
    
    // Waveshaping LFO controls
    wsRateSlider.setSliderStyle(juce::Slider::Rotary);
    wsRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    wsRateSlider.setRange(0.01f, 5.0f, 0.01f);
    wsRateSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    wsRateSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    wsRateSlider.setLookAndFeel(&customDialLookAndFeel);
    addAndMakeVisible(wsRateSlider);

    wsDepthSlider.setSliderStyle(juce::Slider::Rotary);
    wsDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    wsDepthSlider.setRange(0.0f, 1.0f, 0.01f);
    wsDepthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    wsDepthSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    wsDepthSlider.setLookAndFeel(&customDialLookAndFeel);
    addAndMakeVisible(wsDepthSlider);

    wsWaveformSelector.getComboBox().addItem("Sine", 1);
    wsWaveformSelector.getComboBox().addItem("Triangle", 2);
    wsWaveformSelector.getComboBox().addItem("Square", 3);
    wsWaveformSelector.getComboBox().addItem("Saw", 4);
    wsWaveformSelector.getComboBox().setJustificationType(juce::Justification::centred);
    wsWaveformSelector.getComboBox().setLookAndFeel(&customComboBoxLookAndFeel);
    addAndMakeVisible(wsWaveformSelector);

    wsEnableButton.setButtonText("WAVESHAPE");
    wsEnableButton.setLookAndFeel(&customToggleLookAndFeel);
    addAndMakeVisible(wsEnableButton);

    // Create attachments
    wsRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "wsLfoRate", wsRateSlider);
    wsDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "wsLfoDepth", wsDepthSlider);
    wsWaveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "wsLfoWaveform", wsWaveformSelector.getComboBox());
    wsEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "wsEnable", wsEnableButton);
    
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
    
    modRateSlider.setLookAndFeel(nullptr);
    modDepthSlider.setLookAndFeel(nullptr);
    modWaveformSelector.getComboBox().setLookAndFeel(nullptr);
    modTargetSelector.getComboBox().setLookAndFeel(nullptr);
    modEnableButton.setLookAndFeel(nullptr);
    
    wsRateSlider.setLookAndFeel(nullptr);
    wsDepthSlider.setLookAndFeel(nullptr);
    wsWaveformSelector.getComboBox().setLookAndFeel(nullptr);
    wsEnableButton.setLookAndFeel(nullptr);
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
        
        // Get main LFO parameters
        auto waveformParam = audioProcessor.apvts.getRawParameterValue("lfoWaveform");
        auto depthParam = audioProcessor.apvts.getRawParameterValue("lfoDepth");
        auto phaseOffsetParam = audioProcessor.apvts.getRawParameterValue("lfoPhaseOffset");
        auto syncParam = audioProcessor.apvts.getRawParameterValue("lfoSync");
        auto rateParam = audioProcessor.apvts.getRawParameterValue("lfoRate");
        auto divisionParam = audioProcessor.apvts.getRawParameterValue("lfoNoteDivision");

        // Get modulation parameters
        auto modRateParam = audioProcessor.apvts.getRawParameterValue("modLfoRate");
        auto modDepthParam = audioProcessor.apvts.getRawParameterValue("modLfoDepth");
        auto modTargetParam = audioProcessor.apvts.getRawParameterValue("modTarget");

        // Calculate modulation values
        float baseRate = rateParam->load();
        float baseDepth = depthParam->load();
        float basePhase = phaseOffsetParam->load();

        // Get current modulation value from processor
        auto modValue = audioProcessor.getModulationValue();
        auto* modEnableParam = audioProcessor.apvts.getRawParameterValue("modEnable");
        bool modEnabled = modEnableParam->load();
        
        auto* wsEnableParam = audioProcessor.apvts.getRawParameterValue("wsEnable");
        bool wsEnabled = wsEnableParam->load();
        float wsValue = wsEnabled ? audioProcessor.getWaveshapeValue() : 0.0f;

        float modRate = 1.0f, modDepth = 1.0f, modPhase = 0.0f;
        auto target = static_cast<ModulationLFO::Target>(static_cast<int>(modTargetParam->load()));
        
        if (modEnabled)  // Only calculate modulation if enabled
        {
            auto modValue = audioProcessor.getModulationValue();
            
            switch (target)
            {
                case ModulationLFO::Target::Rate:
                    modRate = 0.5f + 1.5f * modValue;
                    break;
                case ModulationLFO::Target::Depth:
                    modDepth = modValue;
                    break;
                case ModulationLFO::Target::Phase:
                    modPhase = (modValue - 0.5f) * juce::MathConstants<float>::pi;
                    break;
            }
        }

        // Update visualizer with all parameters
        lfoVisualizer.setWaveform(static_cast<int>(waveformParam->load()));
        lfoVisualizer.setDepth(baseDepth);
        lfoVisualizer.setPhaseOffset(basePhase);
        lfoVisualizer.setModulationValues(modRate, modDepth, modPhase, target);
        lfoVisualizer.setWaveshapeValues(wsValue, wsEnabled);
        
        // Update rate and sync settings
        if (syncParam->load() > 0.5f)
        {
            lfoVisualizer.setTempoSync(true,
                                     audioProcessor.getCurrentBPM(),
                                     static_cast<int>(divisionParam->load()));
        }
        else
        {
            lfoVisualizer.setRate(baseRate);
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
    g.setColour(juce::Colour(232, 193, 185));  // Light rose gold
    g.setFont(16.0f);

    const int dialSize = 150;
    const int spacing = 20;
    const int totalWidth = (dialSize * 4) + (spacing * 3);
    const int startX = (getWidth() - totalWidth) / 2;
    const int labelY = 210 + dialSize + 5;

    // Main control labels
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
               
    // Modulation section header
    g.setFont(14.0f);
    const int buttonHeight = 40;
    const int comboHeight = 25;
    const int modLabelY = labelY + 95 + buttonHeight + spacing + 5;
    g.drawText("MODULATION",
               juce::Rectangle<int>(0, modLabelY, getWidth(), 20),
               juce::Justification::centred);
               
    // Modulation control labels
    g.setFont(12.0f);
    const int modDialSize = 70;
    const int modSpacing = 15;
    const int modSectionWidth = (modDialSize * 2) + modSpacing + 200 + (modSpacing * 3);
    const int modStartX = (getWidth() - modSectionWidth) / 2;
    const int modLabelY2 = modLabelY + 85;

    g.drawText("MOD RATE",
               juce::Rectangle<int>(modStartX, modLabelY2, modDialSize, 20),
               juce::Justification::centred);
               
    g.drawText("MOD DEPTH",
               juce::Rectangle<int>(modStartX + modDialSize + modSpacing, modLabelY2, modDialSize, 20),
               juce::Justification::centred);

    // Waveshaping section header and labels
    const int wsLabelY = modLabelY2 + modDialSize + spacing * 3;
    g.drawText("WAVESHAPING",
               juce::Rectangle<int>(0, wsLabelY, getWidth(), 20),
               juce::Justification::centred);

    const int wsLabelY2 = wsLabelY + 85;
    g.drawText("WS RATE",
               juce::Rectangle<int>(modStartX, wsLabelY2, modDialSize, 20),
               juce::Justification::centred);
               
    g.drawText("WS DEPTH",
               juce::Rectangle<int>(modStartX + modDialSize + modSpacing, wsLabelY2, modDialSize, 20),
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
    
    // Calculate sizes for main dials
    const int dialSize = 150;
    const int spacing = 20;
    const int totalWidth = (dialSize * 4) + (spacing * 3);
    const int startX = (getWidth() - totalWidth) / 2;
    const int startY = visualizerBounds.getBottom() + spacing;

    // Position main dials
    lfoRateSlider.setBounds(startX, startY, dialSize, dialSize);
    lfoDepthSlider.setBounds(startX + dialSize + spacing, startY, dialSize, dialSize);
    lfoPhaseOffsetSlider.setBounds(startX + (dialSize + spacing) * 2, startY, dialSize, dialSize);
    mixSlider.setBounds(startX + (dialSize + spacing) * 3, startY, dialSize, dialSize);

    // ComboBox dimensions for main controls
    const int comboBoxWidth = 140;
    const int comboBoxHeight = 25;
    const int comboY = startY + dialSize + spacing + 25;

    // Center combo boxes under their dials
    const int rateDialCenterX = startX + (dialSize / 2);
    const int mixDialCenterX = startX + (dialSize + spacing) * 3 + (dialSize / 2);
    
    divisionSelector.setBounds(rateDialCenterX - (comboBoxWidth / 2), comboY, comboBoxWidth, comboBoxHeight);
    waveformSelector.setBounds(mixDialCenterX - (comboBoxWidth / 2), comboY, comboBoxWidth, comboBoxHeight);

    // Button dimensions
    const int buttonWidth = 100;
    const int buttonHeight = 40;
    const int buttonsY = comboY + comboBoxHeight + spacing;

    lfoSyncButton.setBounds(rateDialCenterX - (buttonWidth / 2), buttonsY, buttonWidth, buttonHeight);
    bypassButton.setBounds(mixDialCenterX - (buttonWidth / 2), buttonsY, buttonWidth, buttonHeight);

    // Modulation section
    const int modDialSize = 70;
    const int modSpacing = 15;
    const int modComboWidth = 100;
    const int modComboHeight = 25;
    const int modButtonWidth = 100;
    const int modButtonHeight = 25;

    const int modSectionWidth = (modDialSize * 2) + modSpacing + (modComboWidth * 2) + (modSpacing * 3);
    const int modStartX = (getWidth() - modSectionWidth) / 2;
    const int modStartY = buttonsY + buttonHeight + spacing * 2;

    // Position modulation controls
    modRateSlider.setBounds(modStartX, modStartY, modDialSize, modDialSize);
    modDepthSlider.setBounds(modStartX + modDialSize + modSpacing, modStartY, modDialSize, modDialSize);
    
    const int modComboY = modStartY + (modDialSize - modComboHeight) / 2;
    modWaveformSelector.setBounds(modStartX + (modDialSize * 2) + (modSpacing * 2),
                                modComboY, modComboWidth, modComboHeight);
    modTargetSelector.setBounds(modStartX + (modDialSize * 2) + (modSpacing * 3) + modComboWidth,
                               modComboY, modComboWidth, modComboHeight);
    modEnableButton.setBounds((getWidth() - modButtonWidth) / 2,
                             modStartY - modButtonHeight - spacing,
                             modButtonWidth, modButtonHeight);

    // Waveshaping section
    const int wsStartY = modStartY + modDialSize + spacing * 3;
    
    // Position waveshaping controls
    wsRateSlider.setBounds(modStartX, wsStartY, modDialSize, modDialSize);
    wsDepthSlider.setBounds(modStartX + modDialSize + modSpacing, wsStartY, modDialSize, modDialSize);
    
    const int wsComboY = wsStartY + (modDialSize - modComboHeight) / 2;
    wsWaveformSelector.setBounds(modStartX + (modDialSize * 2) + (modSpacing * 2),
                              wsComboY, modComboWidth, modComboHeight);
                              
    wsEnableButton.setBounds((getWidth() - modButtonWidth) / 2,
                          wsStartY - modButtonHeight - spacing,
                          modButtonWidth, modButtonHeight);

    // Update window size to accommodate new section
    setSize(getWidth(), wsStartY + modDialSize + spacing * 2);
}
