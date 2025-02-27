/*
  ==============================================================================

    HierarchicalPresetMenu.h
    Created: 26 Feb 2025 9:32:02am
    Author:  Deivids Hvostovs

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "CustomComboBox.h"
#include "CustomToggle.h"
#include "ArrowNavigationComboBox.h"
#include "PerlinNoise.h"
#include "CustomMenuLookAndFeel.h"
#include "TransparentButtonLookAndFeel.h"

// Define the menu IDs
enum MenuIDs {
    SavePresetID = 100000,
    OpenFolderID = 100001,
    PresetIDOffset = 200000  // Preset IDs start from this number
};

/**
 * A component that displays presets in a hierarchical menu structure.
 * Supports nested folders and displays the current preset with modification status.
 */
class HierarchicalPresetMenu : public juce::Component,
                              public juce::Timer,
                              public juce::Button::Listener
{
public:
    HierarchicalPresetMenu(PresetManager& pm);
    ~HierarchicalPresetMenu() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void buttonClicked(juce::Button* button) override;
    
    void updatePresetDisplay();
    
    void navigatePresets(bool goForward);
    
    juce::DrawableButton leftArrowButton{"LeftArrow", juce::DrawableButton::ImageFitted};
    juce::DrawableButton rightArrowButton{"RightArrow", juce::DrawableButton::ImageFitted};
    
    
private:
    PresetManager& presetManager;
    
    // UI Elements
    juce::TextButton mainButton; // Shows current preset name
    std::unique_ptr<juce::PopupMenu> rootMenu;
    
    // Currently displayed preset information
    juce::String currentDisplayName;
    bool isModified = false;
    
    // Styling
    juce::Image backgroundImage;
    const float cornerRadius = 3.0f;
    const juce::Colour textColour = juce::Colour(232, 193, 185).withAlpha(0.8f);
    const juce::Colour backgroundColour = juce::Colours::black.withAlpha(0.2f);
    const juce::Colour borderColour = juce::Colours::white.withAlpha(0.1f);
    
    // Helper methods
    void showRootMenu();
    void savePreset();
    void openPresetFolder();
    juce::PopupMenu buildFolderMenu(const juce::String& folderPath);
    juce::Image createBackgroundImage(int width, int height);
    
    // Menu result callback
    void menuItemSelected(int menuItemID);
    
    CustomMenuLookAndFeel menuLookAndFeel;
    
    std::map<int, juce::String> menuIDToPresetMap;
    int nextMenuID = PresetIDOffset;
    TransparentButtonLookAndFeel transparentButtonLookAndFeel;
    
    juce::Time lastScanTime;   
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HierarchicalPresetMenu)
};
