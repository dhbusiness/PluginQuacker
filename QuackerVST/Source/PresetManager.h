/*
  ==============================================================================

    PresetManager.h
    Created: 21 Feb 2025 7:34:15pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>

class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager();

    // Basic preset data structure
    struct Preset {
        juce::String name;
        juce::String category;
        juce::ValueTree state;
        juce::Time dateCreated;
        
        // Constructor for easy preset creation
        Preset(const juce::String& n, const juce::String& cat,
               const juce::ValueTree& s, juce::Time date = juce::Time::getCurrentTime())
            : name(n), category(cat), state(s), dateCreated(date) {}
    };

    // Core preset operations
    bool savePreset(const juce::String& name, const juce::String& category = "User");
    bool loadPreset(const juce::String& name);
    
    // Preset management
    void initializeDefaultPresets();
    juce::StringArray getPresetNames() const;
    juce::StringArray getCategories() const;
    
    const juce::File& getCurrentPresetDirectory() const { return presetDirectory; }
    
    void scanForPresets();
    
    // Remove any presets with category "Factory"
    void clearFactoryPresets();
    
    // State management helpers
    juce::ValueTree getStateFromXml(const juce::XmlElement& xml);
    std::unique_ptr<juce::XmlElement> getXmlFromState(const juce::ValueTree& state);

    
    juce::StringArray getFactoryPresetNames() const;
    juce::StringArray getUserPresetNames() const;
    juce::String getCurrentPresetName() const { return currentPresetName; }
    
    // New functions for modified preset display
    bool isPresetModified() const;
    juce::String getDisplayedPresetName() const;
    
    juce::String getModifiedDisplayName() const;
    
    juce::String getPresetCategory(const juce::String& presetName) const;
    
    void applyParametersInCorrectOrder();
    
    using PresetLoadedCallback = std::function<void()>;
    void setPresetLoadedCallback(PresetLoadedCallback callback) { onPresetLoaded = callback; }
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::File presetDirectory;
    std::map<juce::String, std::unique_ptr<Preset>> presets;
    
    // Private helper methods
    void createPresetDirectory();
    void loadPresetFromFile(const juce::File& file);
    bool savePresetToFile(const Preset& preset);
    juce::String generateSafeFileName(const juce::String& name);
    
    juce::String currentPresetName = "Default";
    
    // Store a clean copy of the preset state when a preset is loaded.
    // This is used to determine if the preset has been modified.
    juce::ValueTree cleanPresetState;
    
    PresetLoadedCallback onPresetLoaded;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
