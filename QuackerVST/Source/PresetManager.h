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
    
    // Add this inside the PresetManager class declaration

    // Represents a folder in the preset hierarchy
    struct PresetFolder {
        juce::String name;
        std::vector<juce::String> presets;  // Presets in this folder
        std::map<juce::String, PresetFolder> subfolders;  // Nested folders
        
        // Helper to add a preset to this folder
        void addPreset(const juce::String& presetName) {
            presets.push_back(presetName);
        }
        
        // Helper to get or create a subfolder
        PresetFolder& getOrCreateSubfolder(const juce::String& folderName) {
            return subfolders[folderName];
        }
    };

    // Root folders (Factory, User, etc.)
    std::map<juce::String, PresetFolder> presetFolders;

    // Build the folder hierarchy from the preset categories
    void buildFolderHierarchy();

    // Get all factory preset categories
    juce::StringArray getFactoryCategories() const;

    // Get all presets in a specific folder path (e.g., "Factory/Vintage Amps")
    juce::StringArray getPresetsInFolder(const juce::String& folderPath) const;

    // Helper to split a path into components
    static std::vector<juce::String> splitFolderPath(const juce::String& path);

    // Convert category to folder path
    juce::String categoryToFolderPath(const juce::String& category) const;

    // Helper function to get a subfolder by path
    const PresetFolder* getFolderByPath(const juce::String& path) const;
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::File presetDirectory;
    std::map<juce::String, std::unique_ptr<Preset>> presets;
    
    // Private helper methods
    void createPresetDirectory();
    void loadPresetFromFile(const juce::File& file);
    bool savePresetToFile(const Preset& preset);
    void scanDirectory(const juce::File& directory, const juce::String& categoryPrefix);
    juce::String determineCategory(const juce::File& file);
    bool savePresetToFile(const Preset& preset, const juce::File& presetFile);
    juce::String generateSafeFileName(const juce::String& name);
    
    juce::String currentPresetName = "Default";
    
    // Store a clean copy of the preset state when a preset is loaded.
    // This is used to determine if the preset has been modified.
    juce::ValueTree cleanPresetState;
    
    PresetLoadedCallback onPresetLoaded;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
