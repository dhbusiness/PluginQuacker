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
    setSize (800, 650);

    // Generate background only if it hasn't been generated yet
    if (!backgroundGenerated)
    {
        generateBackgroundPattern(800, 650);
        backgroundGenerated = true;
    }
    
    startTimerHz(100); //Starting a timer which updates the GUI
    
    //Adding and init LFO rate and depth control params
    // Setup controls
    lfoRateSlider.setSliderStyle(juce::Slider::Rotary);
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    lfoRateSlider.setSkewFactorFromMidPoint(1.0f); // Center the exponential curve around 1 Hz
    lfoRateSlider.setRange(0.01, 25.0, 0.001);
    lfoRateSlider.setDoubleClickReturnValue(true, 1.0); // Reset to 1 Hz on double-click
    addAndMakeVisible(lfoRateSlider);
    
    // Add a lambda function to format the text display nicely
    lfoRateSlider.setTextValueSuffix(" Hz");
    lfoRateSlider.onValueChange = [this]() {
        float value = lfoRateSlider.getValue();
        if (value < 0.1f) {
            // Show more decimal places for very low frequencies
            lfoRateSlider.setTextValueSuffix(" Hz");
            lfoRateSlider.setNumDecimalPlacesToDisplay(3);
        } else if (value < 1.0f) {
            lfoRateSlider.setTextValueSuffix(" Hz");
            lfoRateSlider.setNumDecimalPlacesToDisplay(2);
        } else {
            lfoRateSlider.setTextValueSuffix(" Hz");
            lfoRateSlider.setNumDecimalPlacesToDisplay(1);
        }
    };

    lfoDepthSlider.setSliderStyle(juce::Slider::Rotary);
    lfoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    lfoDepthSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(lfoDepthSlider);


    
    waveformSelector.getComboBox().addItem("Sine", 1);
    waveformSelector.getComboBox().addItem("Square", 2);
    waveformSelector.getComboBox().addItem("Triangle", 3);
    waveformSelector.getComboBox().addItem("Sawtooth Up", 4);
    waveformSelector.getComboBox().addItem("Sawtooth Down", 5);
    waveformSelector.getComboBox().addItem("Soft Square", 6);
    waveformSelector.getComboBox().addItem("Fender Style", 7);
    waveformSelector.getComboBox().addItem("Wurlitzer Style", 8);
    waveformSelector.getComboBox().addItem("Vox Style", 9);
    waveformSelector.getComboBox().addItem("Magnatone Style", 10);
    waveformSelector.getComboBox().addItem("Pulse Decay", 11);
    waveformSelector.getComboBox().addItem("Bouncing Ball", 12);
    waveformSelector.getComboBox().addItem("Multi Sine", 13);
    waveformSelector.getComboBox().addItem("Optical Style", 14);
    waveformSelector.getComboBox().addItem("Twin Peaks", 15);
    waveformSelector.getComboBox().addItem("Smooth Random", 16);
    waveformSelector.getComboBox().addItem("Guitar Pick", 17);
    waveformSelector.getComboBox().addItem("Vintage Chorus", 18);
    waveformSelector.getComboBox().addItem("Slow Gear", 19);
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
    
    // Create bypass attachment
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "bypass", bypassButton);

    // Initialize waveshaping controls
    waveshapeRateSlider.setSliderStyle(juce::Slider::Rotary);
    waveshapeRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    waveshapeRateSlider.setRange(0.01, 25.0, 0.001);
    waveshapeRateSlider.setSkewFactorFromMidPoint(1.0f);
    waveshapeRateSlider.setDoubleClickReturnValue(true, 1.0);
    addAndMakeVisible(waveshapeRateSlider);

    waveshapeDepthSlider.setSliderStyle(juce::Slider::Rotary);
    waveshapeDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    waveshapeDepthSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(waveshapeDepthSlider);

    // Setup waveshape waveform selector
    waveshapeWaveformSelector.getComboBox().addItem("Sine", 1);
    waveshapeWaveformSelector.getComboBox().addItem("Square", 2);
    waveshapeWaveformSelector.getComboBox().addItem("Triangle", 3);
    waveshapeWaveformSelector.getComboBox().addItem("Sawtooth Up", 4);
    waveshapeWaveformSelector.getComboBox().addItem("Sawtooth Down", 5);
    waveshapeWaveformSelector.getComboBox().addItem("Soft Square", 6);
    waveshapeWaveformSelector.getComboBox().addItem("Fender Style", 7);
    waveshapeWaveformSelector.getComboBox().addItem("Wurlitzer Style", 8);
    waveshapeWaveformSelector.getComboBox().addItem("Vox Style", 9);
    waveshapeWaveformSelector.getComboBox().addItem("Magnatone Style", 10);
    waveshapeWaveformSelector.getComboBox().addItem("Pulse Decay", 11);
    waveshapeWaveformSelector.getComboBox().addItem("Bouncing Ball", 12);
    waveshapeWaveformSelector.getComboBox().addItem("Multi Sine", 13);
    waveshapeWaveformSelector.getComboBox().addItem("Optical Style", 14);
    waveshapeWaveformSelector.getComboBox().addItem("Twin Peaks", 15);
    waveshapeWaveformSelector.getComboBox().addItem("Smooth Random", 16);
    waveshapeWaveformSelector.getComboBox().addItem("Guitar Pick", 17);
    waveshapeWaveformSelector.getComboBox().addItem("Vintage Chorus", 18);
    waveshapeWaveformSelector.getComboBox().addItem("Slow Gear", 19);
    addAndMakeVisible(waveformSelector);
    addAndMakeVisible(waveshapeWaveformSelector);

    waveshapeEnableButton.setButtonText("SHAPE");
    waveshapeEnableButton.setLookAndFeel(&customToggleLookAndFeel);
    addAndMakeVisible(waveshapeEnableButton);

    waveshapeRateSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    waveshapeDepthSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    waveshapeRateSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    waveshapeDepthSlider.setColour(juce::Slider::textBoxTextColourId, textColor);
    
    // Create parameter attachments
    waveshapeRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "waveshapeRate", waveshapeRateSlider);
    waveshapeDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "waveshapeDepth", waveshapeDepthSlider);
    waveshapeWaveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "waveshapeWaveform", waveshapeWaveformSelector.getComboBox());
    waveshapeEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "waveshapeEnabled", waveshapeEnableButton);

    // Apply look and feel
    waveshapeRateSlider.setLookAndFeel(&customDialLookAndFeel);
    waveshapeDepthSlider.setLookAndFeel(&customDialLookAndFeel);
    waveshapeWaveformSelector.getComboBox().setLookAndFeel(&customComboBoxLookAndFeel);
    
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
    
    waveshapeRateSlider.setLookAndFeel(nullptr);
    waveshapeDepthSlider.setLookAndFeel(nullptr);
    waveshapeWaveformSelector.getComboBox().setLookAndFeel(nullptr);
    
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
    
    // Update waveshaping parameters
    auto waveshapeRateParam = audioProcessor.apvts.getRawParameterValue("waveshapeRate");
    auto waveshapeDepthParam = audioProcessor.apvts.getRawParameterValue("waveshapeDepth");
    auto waveshapeWaveformParam = audioProcessor.apvts.getRawParameterValue("waveshapeWaveform");
    auto waveshapeEnabledParam = audioProcessor.apvts.getRawParameterValue("waveshapeEnabled");

    lfoVisualizer.setWaveshapeParameters(
        waveshapeDepthParam->load(),
        waveshapeRateParam->load(),
        static_cast<int>(waveshapeWaveformParam->load()),
        waveshapeEnabledParam->load() > 0.5f
    );
    
    repaint();
}
    
//==============================================================================
void QuackerVSTAudioProcessorEditor::generateBackgroundPattern(int width, int height)
{
    backgroundImage = juce::Image(juce::Image::ARGB, width, height, true);
    juce::Graphics g(backgroundImage);
    
    auto bounds = juce::Rectangle<float>(0, 0, width, height);
    
    // Define the raised area for waveshaping section
    const int waveshapeY = 395;  // Adjusted Y position to match controls
    const int waveshapeHeight = 250;  // Reduced height to match control area
    const int waveshapeMargin = 260;   // Increased margin for better centering
    juce::Rectangle<float> raisedArea(waveshapeMargin,
                                    waveshapeY,
                                    width - (waveshapeMargin * 2),
                                    waveshapeHeight);
    
    // Enhanced color palette with metallic tones
    juce::Colour darkPlum(61, 21, 46);
    juce::Colour midPlum(72, 28, 55);
    juce::Colour lightPlum(89, 34, 68);
    juce::Colour peachPink = juce::Colour(255, 201, 190).withMultipliedBrightness(0.6f);
    juce::Colour roseGold = juce::Colour(232, 193, 185).withMultipliedBrightness(0.6f);
    juce::Colour warmPlum = juce::Colour(198, 109, 139).withMultipliedBrightness(0.7f);
    
    // Metallic sheen colors
    juce::Colour metalHighlight = juce::Colour(255, 255, 255).withAlpha(0.03f);
    juce::Colour metalShadow = juce::Colours::black.withAlpha(0.05f);

    // Base gradient
    juce::ColourGradient baseGradient(
        darkPlum.brighter(0.05f),
        bounds.getCentreX(), bounds.getCentreY(),
        midPlum.darker(0.05f),
        bounds.getRight(), bounds.getBottom(),
        true);
    
    baseGradient.addColour(0.3, midPlum);
    baseGradient.addColour(0.7, lightPlum);
    
    g.setGradientFill(baseGradient);
    g.fillAll();

    // Add initial sheen
    juce::ColourGradient sheenGradient(
        metalHighlight,
        bounds.getCentreX(), bounds.getCentreY(),
        metalShadow,
        bounds.getRight(), bounds.getBottom(),
        true);
    
    g.setGradientFill(sheenGradient);
    g.fillAll();

    // Create more pronounced raised platform effect
    // Softer shadow for depth
    for(int i = 0; i < 12; ++i) {
        float alpha = 0.03f * (12 - i);  // Reduced shadow intensity
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.fillRoundedRectangle(raisedArea.translated(0, i + 2).expanded(2), 8.0f);
    }
    
    // Add side edges for more prominent raised effect
    juce::Path edgePath;
    edgePath.addRoundedRectangle(raisedArea, 8.0f);
    g.setColour(juce::Colours::black.withAlpha(0.4f));  // More subtle edge color
    g.strokePath(edgePath, juce::PathStrokeType(2.5f));  // Slightly thinner edge
    
    // Draw softer black main surface
    g.setColour(juce::Colours::black.withAlpha(0.7f));  // More transparent black
    g.fillRoundedRectangle(raisedArea, 8.0f);
    
    // Add more subtle glossy highlights
    juce::ColourGradient glossGradient(
        juce::Colours::white.withAlpha(0.08f),  // Reduced highlight intensity
        raisedArea.getTopLeft(),
        juce::Colours::transparentBlack,
        raisedArea.getTopLeft().translated(0, raisedArea.getHeight() * 0.4f),
        false);
    g.setGradientFill(glossGradient);
    g.fillRoundedRectangle(raisedArea, 8.0f);
    
    // Softer light reflection on edges
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(raisedArea.reduced(1), 8.0f, 1.0f);
    
    // More subtle specular highlight
    juce::Rectangle<float> specularArea = raisedArea.reduced(20);
    juce::ColourGradient specularGradient(
        juce::Colours::white.withAlpha(0.03f),
        specularArea.getCentreX(), specularArea.getY(),
        juce::Colours::transparentBlack,
        specularArea.getCentreX(), specularArea.getCentreY(),
        true);
    g.setGradientFill(specularGradient);
    g.fillRoundedRectangle(specularArea, 6.0f);

    // Enhanced metal flakes
    juce::Random random;
    for (int i = 0; i < 15000; ++i)
    {
        float x = random.nextFloat() * width;
        float y = random.nextFloat() * height;
        float size = random.nextFloat() * 1.8f;
        float alpha = random.nextFloat() * 0.08f;

        // Reduced flake intensity in raised area
        if (raisedArea.contains(x, y)) {
            alpha *= 0.6f;  // Reduced from 1.5f to 0.6f
            size *= 0.8f;   // Reduced from 1.2f to 0.8f
        }

        if (random.nextBool()) {
            g.setColour(juce::Colours::white.withAlpha(alpha));
        } else {
            g.setColour(metalShadow.withAlpha(alpha * 0.8f));
        }
        
        g.fillEllipse(x, y, size, size);
    }

    // Add Perlin noise patterns
    const float scale = 0.007f;
    const float fixedSeed = 42.0f;
    
    struct LayerConfig {
        float scale;
        float alpha;
        float amplitude;
        juce::Colour color;
        float offset;
    };

    std::array<LayerConfig, 5> layers = {{
        { 0.003f, 0.08f, 8.0f, warmPlum, 0.0f },
        { 0.005f, 0.06f, 6.0f, roseGold, 50.0f },
        { 0.008f, 0.04f, 5.0f, peachPink, 100.0f },
        { 0.002f, 0.08f, 10.0f, lightPlum, 150.0f },
        { 0.004f, 0.03f, 6.0f, warmPlum, 200.0f }
    }};

    // Generate texture layers with raised area emphasis
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
                
                // Enhance effect in raised area
                if (raisedArea.contains(x, y)) {
                    offsetX *= 1.2f;
                    offsetY *= 1.2f;
                }
                
                swirlyPath.lineTo(x + offsetX, y + offsetY);
            }
        }
        
        float alpha = layer.alpha;
        if (raisedArea.contains(swirlyPath.getBounds().getCentre())) {
            alpha *= 1.2f;
        }
        
        g.setColour(layer.color.withAlpha(alpha));
        g.strokePath(swirlyPath, juce::PathStrokeType(1.5f));
    }

    // Final highlight for raised area
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::white.withAlpha(0.05f),
        raisedArea.getCentre(),
        juce::Colours::transparentBlack,
        raisedArea.getTopLeft(),
        true));
    g.fillRoundedRectangle(raisedArea, 8.0f);

    // Final overall sheen
    juce::ColourGradient finalSheen(
        metalHighlight.withAlpha(0.01f),
        bounds.getCentreX(), bounds.getCentreY(),
        metalShadow.withAlpha(0.01f),
        bounds.getRight(), bounds.getBottom(),
        true);
    
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
    const int dialSize = 150;
    const int spacing = 20;
    const int totalWidth = (dialSize * 4) + (spacing * 3);
    const int startX = (getWidth() - totalWidth) / 2;
    const int labelY = 210 + dialSize + 5;

    // Define colors for embossed effect
    juce::Colour textColor = juce::Colour(232, 193, 185);  // Rose gold base
    
    // Function to draw embossed text
    auto drawEmbossedText = [&](const juce::String& text, const juce::Rectangle<int>& bounds) {
        g.setFont(16.0f);
        
        // Bottom highlight (creates depth)
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawText(text, bounds.translated(0, 1), juce::Justification::centred, false);
        
        // Top shadow (creates inset effect)
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.drawText(text, bounds.translated(0, -1), juce::Justification::centred, false);
        
        // Main text
        g.setColour(textColor.withAlpha(0.8f));  // Slightly transparent to enhance inset effect
        g.drawText(text, bounds, juce::Justification::centred, false);
    };

    // Draw all labels with the embossed style
    drawEmbossedText("RATE",
                     juce::Rectangle<int>(startX, labelY, dialSize, 20));
               
    drawEmbossedText("DEPTH",
                     juce::Rectangle<int>(startX + dialSize + spacing, labelY, dialSize, 20));
               
    drawEmbossedText("WAVE OFFSET",
                     juce::Rectangle<int>(startX + (dialSize + spacing) * 2, labelY, dialSize, 20));
               
    drawEmbossedText("MIX",
                     juce::Rectangle<int>(startX + (dialSize + spacing) * 3, labelY, dialSize, 20));

    // Waveshaping controls text
    const int smallDialSize = dialSize * 0.75;
    const int waveshapeControlsWidth = smallDialSize * 2 + spacing;
    const int waveshapeStartX = (getWidth() - waveshapeControlsWidth) / 2;
    
    drawEmbossedText("SHAPE RATE",
                     juce::Rectangle<int>(waveshapeStartX,
                                        waveshapeRateSlider.getBottom(),
                                        smallDialSize,
                                        20));
               
    drawEmbossedText("SHAPE DEPTH",
                     juce::Rectangle<int>(waveshapeStartX + smallDialSize + spacing,
                                        waveshapeDepthSlider.getBottom(),
                                        smallDialSize,
                                        20));
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
    
    auto mainControlsArea = bounds.removeFromTop(dialSize + 100);
    
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
    
    // Waveshaping controls section
    const int smallDialSize = dialSize * 0.75;
    const int waveshapeControlsWidth = smallDialSize * 2 + spacing;
    const int waveshapeStartX = (getWidth() - waveshapeControlsWidth) / 2;
    
    // Align waveshape dials with the combo boxes
    const int waveshapeY = comboY;  // Align top with combobox
    
    // Position waveshaping controls
    waveshapeRateSlider.setBounds(waveshapeStartX,
                                 waveshapeY,
                                 smallDialSize,
                                 smallDialSize);
                                 
    waveshapeDepthSlider.setBounds(waveshapeStartX + smallDialSize + spacing,
                                  waveshapeY,
                                  smallDialSize,
                                  smallDialSize);
                                  
    // Position selector and button below the dials with updated spacing
    const int selectorWidth = 140;
    const int selectorHeight = 25;
    const int selectorSpacing = spacing * 1.5;
    
    waveshapeWaveformSelector.setBounds(waveshapeStartX + (waveshapeControlsWidth - selectorWidth) / 2,
                                      waveshapeY + smallDialSize + selectorSpacing,
                                      selectorWidth,
                                      selectorHeight);
                                      
    waveshapeEnableButton.setBounds(waveshapeStartX + (waveshapeControlsWidth - buttonWidth) / 2,
                                    waveshapeY + smallDialSize + selectorSpacing * 2 + (spacing - 5),
                                   buttonWidth,
                                   buttonHeight);
}
