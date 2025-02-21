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
                       public juce::Button::Listener,
                       public juce::ComboBox::Listener
{
public:
    PresetComponent(PresetManager& pm);
    ~PresetComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Button::Listener implementation
    void buttonClicked(juce::Button* button) override;

    // ComboBox::Listener implementation
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    // Update the preset list
    void updatePresetList();

private:
    // Reference to our PresetManager
    PresetManager& presetManager;

    // UI Components
    ArrowNavigationComboBox presetSelector;
    juce::TextButton saveButton;
    
    // Popup menu for saving presets
    void showSavePresetDialog();


    // Helper methods
    void loadSelectedPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetComponent)
};
