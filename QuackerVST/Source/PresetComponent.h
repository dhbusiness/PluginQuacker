/*
  ==============================================================================

    PresetComponent.h
    Created: 21 Feb 2025 7:40:21pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/


#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "CustomComboBox.h"
#include "CustomToggle.h"
#include "ArrowNavigationComboBox.h"  // Added this include


class PresetComponent : public juce::Component,
                       public juce::ComboBox::Listener
{
public:
    PresetComponent(PresetManager& pm);
    ~PresetComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void updatePresetList();

private:
    PresetManager& presetManager;
    ArrowNavigationComboBox presetSelector;
    
    // Styling constants
    const float cornerRadius = 3.0f;  // Reduced corner radius for more minimal look
    const juce::Colour textColour = juce::Colour(232, 193, 185).withAlpha(0.8f);
    const juce::Colour backgroundColour = juce::Colours::black.withAlpha(0.2f);
    const juce::Colour borderColour = juce::Colours::white.withAlpha(0.1f);
    
    void showSavePresetDialog();
    void loadSelectedPreset();

    
    class PresetSelectorLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                         int buttonX, int buttonY, int buttonW, int buttonH,
                         juce::ComboBox& box) override
        {
            // Draw nothing - this gives us a completely transparent background
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            // Position the text in the center of the entire combo box
            label.setBounds(0, 0, box.getWidth(), box.getHeight());
            label.setJustificationType(juce::Justification::centred);
        }
    };

    private:
        
        PresetSelectorLookAndFeel presetLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetComponent)
};
