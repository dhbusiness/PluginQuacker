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
    // Cache current parameter values
    auto* bypassParam = audioProcessor.apvts.getRawParameterValue("bypass");
    auto* waveformParam = audioProcessor.apvts.getRawParameterValue("lfoWaveform");
    auto* depthParam = audioProcessor.apvts.getRawParameterValue("lfoDepth");
    auto* phaseOffsetParam = audioProcessor.apvts.getRawParameterValue("lfoPhaseOffset");
    auto* syncParam = audioProcessor.apvts.getRawParameterValue("lfoSync");
    auto* rateParam = audioProcessor.apvts.getRawParameterValue("lfoRate");
    
    bool isBypassed = bypassParam->load();
    bool parametersChanged = false;
    
    // Check if any parameters have changed
    if (lastBypass != isBypassed ||
        lastWaveform != static_cast<int>(waveformParam->load()) ||
        lastDepth != depthParam->load() ||
        lastPhaseOffset != phaseOffsetParam->load() ||
        lastSync != (syncParam->load() > 0.5f) ||
        lastRate != rateParam->load())
    {
        parametersChanged = true;
        
        // Update cached values
        lastBypass = isBypassed;
        lastWaveform = static_cast<int>(waveformParam->load());
        lastDepth = depthParam->load();
        lastPhaseOffset = phaseOffsetParam->load();
        lastSync = (syncParam->load() > 0.5f);
        lastRate = rateParam->load();
    }

    if (!isBypassed)
    {
        bool isActive = audioProcessor.isPlaying() && audioProcessor.hasAudioInput();
        lfoVisualizer.setActive(isActive, audioProcessor.isLfoWaitingForReset());
        
        if (parametersChanged)
        {
            // Only update visualizer parameters if they've changed
            lfoVisualizer.setWaveform(lastWaveform);
            lfoVisualizer.setDepth(lastDepth);
            lfoVisualizer.setPhaseOffset(lastPhaseOffset);
            
            if (lastSync)
            {
                lfoVisualizer.setTempoSync(true,
                                         audioProcessor.getCurrentBPM(),
                                         static_cast<int>(audioProcessor.apvts.getRawParameterValue("lfoNoteDivision")->load()));
            }
            else
            {
                lfoVisualizer.setRate(lastRate);
            }
        }
    }
    else
    {
        lfoVisualizer.setActive(false, false);
    }
    
    // Only repaint if parameters changed
    if (parametersChanged)
    {
        repaint();
    }
}
    
//==============================================================================
void QuackerVSTAudioProcessorEditor::generateBackgroundPattern(int width, int height)
{
    // Create image with hardware acceleration if available
    backgroundImage = juce::Image(juce::Image::PixelFormat::ARGB, width, height, true);
    juce::Graphics g(backgroundImage);
    
    auto bounds = juce::Rectangle<float>(0, 0, width, height);
    
    // Pre-calculate and cache colors
    const auto darkPlum = juce::Colour(61, 21, 46);
    const auto midPlum = juce::Colour(72, 28, 55);
    const auto lightPlum = juce::Colour(89, 34, 68);
    const auto peachPink = juce::Colour(255, 201, 190).withMultipliedBrightness(0.6f);
    const auto roseGold = juce::Colour(232, 193, 185).withMultipliedBrightness(0.6f);
    const auto warmPlum = juce::Colour(198, 109, 139).withMultipliedBrightness(0.7f);
    const auto metalHighlight = juce::Colour(255, 255, 255).withAlpha(0.03f);
    const auto metalShadow = juce::Colours::black.withAlpha(0.05f);

    // Create single base gradient
    auto baseGradient = juce::ColourGradient(
        darkPlum.brighter(0.05f),
        bounds.getCentreX(), bounds.getCentreY(),
        midPlum.darker(0.05f),
        bounds.getRight(), bounds.getBottom(),
        true);
    
    baseGradient.addColour(0.3, midPlum);
    baseGradient.addColour(0.7, lightPlum);
    
    g.setGradientFill(baseGradient);
    g.fillAll();

    // Pre-calculate flake positions and properties
    struct Flake {
        float x, y, size, alpha;
        bool isHighlight;
    };
    
    std::vector<Flake> flakes;
    flakes.reserve(15000);
    juce::Random random;
    
    for (int i = 0; i < 15000; ++i)
    {
        flakes.push_back({
            random.nextFloat() * width,
            random.nextFloat() * height,
            random.nextFloat() * 1.8f,
            random.nextFloat() * 0.08f,
            random.nextBool()
        });
    }

    // Draw flakes in batches
    const int batchSize = 1000;
    for (int i = 0; i < flakes.size(); i += batchSize)
    {
        for (int j = i; j < std::min(i + batchSize, static_cast<int>(flakes.size())); ++j)
        {
            const auto& flake = flakes[j];
            g.setColour(flake.isHighlight ?
                       juce::Colours::white.withAlpha(flake.alpha) :
                       metalShadow.withAlpha(flake.alpha * 0.8f));
            g.fillEllipse(flake.x, flake.y, flake.size, flake.size);
        }
    }

    // Pre-calculate noise values for efficiency
    const float scale = 0.007f;
    const float fixedSeed = 42.0f;
    std::vector<float> noiseCache;
    const int noiseStepSize = 4;
    const int noiseWidth = static_cast<int>(std::ceil(width / noiseStepSize)) + 1;
    const int noiseHeight = static_cast<int>(std::ceil(height / noiseStepSize)) + 1;
    noiseCache.resize(noiseWidth * noiseHeight);

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < noiseHeight; ++y)
    {
        for (int x = 0; x < noiseWidth; ++x)
        {
            float xPos = x * noiseStepSize;
            float yPos = y * noiseStepSize;
            noiseCache[y * noiseWidth + x] = PerlinNoise::noise(
                xPos * scale, yPos * scale, fixedSeed);
        }
    }

    // Optimized layer configuration
    const std::array<LayerConfig, 5> layers = {{
        { 0.003f, 0.08f, 8.0f, warmPlum, 0.0f },
        { 0.005f, 0.06f, 6.0f, roseGold, 50.0f },
        { 0.008f, 0.04f, 5.0f, peachPink, 100.0f },
        { 0.002f, 0.08f, 10.0f, lightPlum, 150.0f },
        { 0.004f, 0.03f, 6.0f, warmPlum, 200.0f }
    }};

    // Draw layers using pre-calculated noise
    for (const auto& layer : layers)
    {
        juce::Path swirlyPath;
        bool pathStarted = false;
        
        for (int y = 0; y < noiseHeight - 1; ++y)
        {
            for (int x = 0; x < noiseWidth - 1; ++x)
            {
                float noise = noiseCache[y * noiseWidth + x];
                float combinedNoise = noise * juce::MathConstants<float>::pi * 4;
                
                float xPos = x * noiseStepSize + std::sin(combinedNoise) * layer.amplitude;
                float yPos = y * noiseStepSize + std::cos(combinedNoise) * layer.amplitude;
                
                if (!pathStarted)
                {
                    swirlyPath.startNewSubPath(xPos, yPos);
                    pathStarted = true;
                }
                else
                {
                    swirlyPath.lineTo(xPos, yPos);
                }
            }
        }
        
        g.setColour(layer.color.withAlpha(layer.alpha));
        g.strokePath(swirlyPath, juce::PathStrokeType(1.5f));
    }

    // Final metallic sheen with single gradient
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
