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
    // Error codes for preset operations
    enum class ErrorCode {
        None = 0,
        DirectoryCreationFailed,
        FileWriteFailed,
        FileReadFailed,
        InvalidPresetData,
        PresetNotFound,
        InvalidPresetName,
        InvalidCategory,
        ParameterError
    };
    
    PresetManager(juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager();

    // Basic preset data structure with validation
    struct Preset {
        juce::String name;
        juce::String category;
        juce::ValueTree state;
        juce::Time dateCreated;
        
        // Constructor with validation
        Preset(const juce::String& n, const juce::String& cat,
               const juce::ValueTree& s, juce::Time date = juce::Time::getCurrentTime())
            : name(sanitizeName(n)),
              category(sanitizeCategory(cat)),
              state(s),
              dateCreated(date) {}
        
        // Validation helpers
        static juce::String sanitizeName(const juce::String& name);
        static juce::String sanitizeCategory(const juce::String& category);
        bool isValid() const noexcept;
    };

    // Core preset operations with error handling
    bool savePreset(const juce::String& name, const juce::String& category = "User");
    bool loadPreset(const juce::String& name);
    ErrorCode getLastError() const noexcept { return lastError; }
    juce::String getLastErrorMessage() const noexcept { return lastErrorMessage; }
    void clearError() noexcept { lastError = ErrorCode::None; lastErrorMessage.clear(); }
    
    // Preset management
    void initializeDefaultPresets();
    juce::StringArray getPresetNames() const;
    juce::StringArray getCategories() const;
    
    const juce::File& getCurrentPresetDirectory() const { return presetDirectory; }
    
    void scanForPresets();
    void clearFactoryPresets();
    
    // State management helpers
    juce::ValueTree getStateFromXml(const juce::XmlElement& xml);
    std::unique_ptr<juce::XmlElement> getXmlFromState(const juce::ValueTree& state);
    
    juce::StringArray getFactoryPresetNames() const;
    juce::StringArray getUserPresetNames() const;
    juce::String getCurrentPresetName() const { return currentPresetName; }
    
    // Modified preset display
    bool isPresetModified() const noexcept;
    juce::String getDisplayedPresetName() const;
    juce::String getModifiedDisplayName() const;
    juce::String getPresetCategory(const juce::String& presetName) const;
    
    void applyParametersInCorrectOrder();
    
    using PresetLoadedCallback = std::function<void()>;
    void setPresetLoadedCallback(PresetLoadedCallback callback) { onPresetLoaded = callback; }
    
    // Folder hierarchy representation
    struct PresetFolder {
        juce::String name;
        std::vector<juce::String> presets;
        std::map<juce::String, PresetFolder> subfolders;
        
        void addPreset(const juce::String& presetName) {
            if (presetName.isNotEmpty() && !contains(presetName)) {
                presets.push_back(presetName);
            }
        }
        
        PresetFolder& getOrCreateSubfolder(const juce::String& folderName) {
            if (folderName.isNotEmpty()) {
                return subfolders[folderName];
            }
            return *this;
        }
        
        bool contains(const juce::String& presetName) const {
            return std::find(presets.begin(), presets.end(), presetName) != presets.end();
        }
    };

    // Root folders (Factory, User, etc.)
    std::map<juce::String, PresetFolder> presetFolders;

    // Build the folder hierarchy from the preset categories
    void buildFolderHierarchy();

    // Get all factory preset categories
    juce::StringArray getFactoryCategories() const;

    // Get all presets in a specific folder path
    juce::StringArray getPresetsInFolder(const juce::String& folderPath) const;

    // Helper to split a path into components
    static std::vector<juce::String> splitFolderPath(const juce::String& path);

    // Convert category to folder path
    juce::String categoryToFolderPath(const juce::String& category) const;

    // Helper function to get a subfolder by path
    const PresetFolder* getFolderByPath(const juce::String& path) const;
    
    void setCustomPresetName(const juce::String& name);
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::File presetDirectory;
    std::map<juce::String, std::unique_ptr<Preset>> presets;
    
    // Thread safety
    mutable juce::ReadWriteLock presetsLock;
    
    // Error handling
    mutable ErrorCode lastError = ErrorCode::None;
    mutable juce::String lastErrorMessage;
    
    // Private helper methods with error handling
    bool createPresetDirectory();
    bool loadPresetFromFile(const juce::File& file);
    bool savePresetToFile(const Preset& preset);
    bool savePresetToFile(const Preset& preset, const juce::File& presetFile);
    void scanDirectory(const juce::File& directory, const juce::String& categoryPrefix);
    juce::String determineCategory(const juce::File& file);
    juce::String generateSafeFileName(const juce::String& name);
    
    // Validation
    bool validatePresetName(const juce::String& name) const noexcept;
    bool validateCategory(const juce::String& category) const noexcept;
    bool validatePresetFile(const juce::File& file) const noexcept;
    
    // Error reporting
    void reportError(ErrorCode code, const juce::String& message) const noexcept;
    
    juce::String currentPresetName = "Default";
    
    // Store a clean copy of the preset state when a preset is loaded
    juce::ValueTree cleanPresetState;
    
    PresetLoadedCallback onPresetLoaded;
    
    // Constants
    static constexpr size_t MAX_PRESET_NAME_LENGTH = 128;
    static constexpr size_t MAX_CATEGORY_LENGTH = 256;
    static constexpr juce::int64 MAX_PRESET_FILE_SIZE = 1024 * 1024; // 1MB limit
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
