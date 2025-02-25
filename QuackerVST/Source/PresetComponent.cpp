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
    // Get reference to the combo box
    auto& combo = presetSelector.getComboBox();
    
    // Apply our custom LookAndFeel just to this combo box
    combo.setLookAndFeel(&presetLookAndFeel);
    
    combo.addListener(this);
    combo.setTextWhenNothingSelected("Default");
    combo.setJustificationType(juce::Justification::centred);
    
    // Keep only the essential colors, removing backgrounds
    combo.setColour(juce::ComboBox::textColourId, juce::Colour(232, 193, 185));
    
    addAndMakeVisible(presetSelector);
    updatePresetList();

    // Start a timer to update the preset name display every 100ms
    startTimer(100);
}

PresetComponent::~PresetComponent()
{
    presetSelector.getComboBox().setLookAndFeel(nullptr);
    presetSelector.getComboBox().removeListener(this);
}

void PresetComponent::paint(juce::Graphics& g)
{
    // Optionally, draw a background or borders here.
    
    // Draw the combo box normally (its text remains unmodified).
    // Then, if the current preset is modified, overlay an asterisk.
    if (presetManager.isPresetModified())
    {
        // Get the bounds of the combo box
        auto comboBounds = presetSelector.getComboBox().getBounds();

        // Adjust horizontal offset: starWidth is the width of the asterisk area, and offsetX is how far from the right edge.
        int starWidth = 12;
        int offsetX = comboBounds.getRight() - starWidth + 277.5;
        
        // Create a rectangle for the asterisk
        juce::Rectangle<int> starBounds(offsetX, comboBounds.getY(), starWidth, comboBounds.getHeight());
        
        juce::Colour textColor = juce::Colour(232, 193, 185);  // Rose gold base
        g.setColour(textColor); // Color of the asterisk.
        g.setFont(16.0f); // Font size for the asterisk.
        g.drawFittedText("*", starBounds, juce::Justification::centred, 1);
    }
}

void PresetComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Add horizontal margins but keep full height
    const int horizontalMargin = bounds.getWidth() * 0.35; // 35% margin on each side
    bounds.reduce(horizontalMargin, 0);
    
    presetSelector.setBounds(bounds);
}

void PresetComponent::timerCallback()
{
    repaint();
}

void PresetComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetSelector.getComboBox())
    {
        auto selectedItem = presetSelector.getComboBox().getText();
        
        if (selectedItem == "Save Current...")
        {
            showSavePresetDialog();
            // Reset selection to previous preset after showing dialog
            presetSelector.getComboBox().setText(presetManager.getDisplayedPresetName(),
                juce::dontSendNotification);
        }
        else if (selectedItem == "Open Preset Folder")
        {
            juce::File presetDir = presetManager.getCurrentPresetDirectory();
            if (presetDir.exists())
                presetDir.revealToUser();
                
            // Reset selection
            presetSelector.getComboBox().setText(presetManager.getDisplayedPresetName(),
                juce::dontSendNotification);
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

    // Add Factory Presets section
    auto factoryPresets = presetManager.getFactoryPresetNames();
    if (!factoryPresets.isEmpty())
    {
        combo.addSectionHeading("Factory Presets");
        // Ensure "Default" is first if it exists
        if (factoryPresets.contains("Default"))
        {
            combo.addItem("Default", index++);
            factoryPresets.removeString("Default");
        }
        for (const auto& preset : factoryPresets)
        {
            combo.addItem(preset, index++);
        }
    }

    // Add User Presets section
    auto userPresets = presetManager.getUserPresetNames();
    if (!userPresets.isEmpty())
    {
        combo.addSectionHeading("User Presets");
        for (const auto& preset : userPresets)
        {
            combo.addItem(preset, index++);
        }
    }

    // Add Utility section
    combo.addSectionHeading("Utility");
    combo.addItem("Save Current...", index++);
    combo.addItem("Open Preset Folder", index++);

    // Set initial selection to "Default" if it exists
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
        [this, window](int result)
        {
            if (result == 1) // Save button was clicked
            {
                juce::String presetName = window->getTextEditorContents("presetName");
                if (presetName.isNotEmpty())
                {
                    if (presetManager.savePreset(presetName))
                    {
                        updatePresetList();
                        // Set the combo box text to the newly saved preset's name
                        presetSelector.getComboBox().setText(presetName, juce::dontSendNotification);
                        // Load the new preset so that the * is not present
                        presetManager.loadPreset(presetName);
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
