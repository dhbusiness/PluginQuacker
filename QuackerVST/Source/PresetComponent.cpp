/*
  ==============================================================================

    PresetComponent.cpp
    Created: 21 Feb 2025 7:40:21pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/


#include "PresetComponent.h"

PresetComponent::PresetComponent(PresetManager& pm)
    : presetManager(pm)
{
    // Initialize and style the preset selector
    presetSelector.getComboBox().addListener(this);
    presetSelector.getComboBox().setTextWhenNothingSelected("Select Preset");
    addAndMakeVisible(presetSelector);

    // Initialize and style the save button
    saveButton.setButtonText("Save");
    saveButton.addListener(this);
    saveButton.setColour(juce::TextButton::buttonColourId, juce::Colour(19, 224, 139).withAlpha(0.2f));
    saveButton.setColour(juce::TextButton::textColourOffId, juce::Colour(232, 193, 185));
    addAndMakeVisible(saveButton);



    // Initial population of the preset list
    updatePresetList();
}

PresetComponent::~PresetComponent()
{
    presetSelector.getComboBox().removeListener(this);
    saveButton.removeListener(this);
}

void PresetComponent::paint(juce::Graphics& g)
{
    // Background with a slight gradient
    auto bounds = getLocalBounds().toFloat();
    
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::black.withAlpha(0.7f),
        bounds.getTopLeft(),
        juce::Colours::black.withAlpha(0.5f),
        bounds.getBottomLeft(),
        false));
    
    g.fillRoundedRectangle(bounds, 6.0f);

    // Add a subtle border
    g.setColour(juce::Colour(171, 136, 132).withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void PresetComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);
    const int buttonWidth = 60;
    const int spacing = 4;

    // Layout the components
    presetSelector.setBounds(bounds.removeFromLeft(bounds.getWidth() - (buttonWidth * 2 + spacing * 2)));
    bounds.removeFromLeft(spacing);
    saveButton.setBounds(bounds.removeFromLeft(buttonWidth));
    bounds.removeFromLeft(spacing);

}

void PresetComponent::buttonClicked(juce::Button* button)
{
    if (button == &saveButton)
    {
        showSavePresetDialog();
    }
}

void PresetComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetSelector.getComboBox())
    {
        auto selectedPreset = presetSelector.getComboBox().getText();
        if (selectedPreset == "Directory")
        {
            // Open the directory containing the presets
            juce::File presetDir = presetManager.getCurrentPresetDirectory();
            if (presetDir.exists())
                presetDir.revealToUser(); // This will open the folder in the OS file explorer
            
            // Optionally, reset the selection back to "Default"
            presetSelector.getComboBox().setText("Default", juce::dontSendNotification);
        }
        else
        {
            loadSelectedPreset();
        }
    }
}


void PresetComponent::updatePresetList()
{
    auto& combo = presetSelector.getComboBox();
    combo.clear();
    int index = 1;

    // Factory Presets: Ensure "Default" is at the top.
    auto factoryPresets = presetManager.getFactoryPresetNames();
    if (factoryPresets.contains("Default"))
    {
        combo.addSectionHeading("Factory Presets");
        combo.addItem("Default", index++);
        factoryPresets.removeString("Default");
        for (const auto& preset : factoryPresets)
            combo.addItem(preset, index++);
    }
    else if (factoryPresets.size() > 0)
    {
        combo.addSectionHeading("Factory Presets");
        for (const auto& preset : factoryPresets)
            combo.addItem(preset, index++);
    }

    // User Presets (if any)
    auto userPresets = presetManager.getUserPresetNames();
    if (userPresets.size() > 0)
    {
        combo.addSectionHeading("User Presets");
        for (const auto& preset : userPresets)
            combo.addItem(preset, index++);
    }

    // Add the Directory option at the end.
    combo.addSectionHeading("Extras");
    combo.addItem("Directory", index++);

    // Set default selection to "Default"
    combo.setText("Default", juce::dontSendNotification);
}




void PresetComponent::showSavePresetDialog()
{
    // Create a custom alert window for saving presets
    auto* window = new juce::AlertWindow("Save Preset",
                                       "Enter a name for your preset:",
                                       juce::MessageBoxIconType::NoIcon);
    
    // Add a text editor for the preset name
    window->addTextEditor("presetName", "", "Preset Name:");
    
    // Add buttons
    window->addButton("Save", 1);
    window->addButton("Cancel", 0);

    // Show the window asynchronously
    window->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, window](int result) {
            if (result == 1) // Save button was clicked
            {
                juce::String presetName = window->getTextEditorContents("presetName");
                if (presetName.isNotEmpty())
                {
                    if (presetManager.savePreset(presetName))
                    {
                        updatePresetList();
                        presetSelector.getComboBox().setSelectedId(
                            presetSelector.getComboBox().getNumItems(),
                            juce::sendNotification);
                    }
                }
            }
            delete window; // Clean up the window
        }));
}


void PresetComponent::loadSelectedPreset()
{
    auto selectedPreset = presetSelector.getComboBox().getText();
    if (selectedPreset.isNotEmpty())
    {
        presetManager.loadPreset(selectedPreset);
    }
}
