/*
  ==============================================================================

    PresetManagerTests.cpp
    Created: Unit Tests for PresetManager Class
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "PresetManagerTests.h"
#include "../QuackerVST/Source/PresetManager.h"
#include "../QuackerVST/Source/PluginProcessor.h"

void PresetManagerTests::runTest()
{
    beginTest("Initialization");
    testInitialization();
    
    beginTest("Preset Scanning");
    testPresetScanning();
    
    beginTest("Preset Loading");
    testPresetLoading();
    
    beginTest("Preset Saving");
    testPresetSaving();
    
    beginTest("Folder Hierarchy");
    testFolderHierarchy();
    
    beginTest("Preset Modification");
    testPresetModification();
    
    beginTest("Error Handling");
    testErrorHandling();
    
    beginTest("Factory Presets");
    testFactoryPresets();
}

void PresetManagerTests::testInitialization()
{
    auto apvts = createTestAPVTS();
    expect(apvts != nullptr, "Test APVTS should be created");
    
    try
    {
        PresetManager presetManager(*apvts);
        
        // Test basic initialization
        expect(presetManager.getLastError() == PresetManager::ErrorCode::None, 
               "PresetManager should initialize without errors");
        
        // Test default directory creation
        auto presetDir = presetManager.getCurrentPresetDirectory();
        expect(presetDir.exists(), "Preset directory should be created during initialization");
        
        logMessage("✓ PresetManager initialization successful");
    }
    catch (const std::exception& e)
    {
        expect(false, "PresetManager initialization threw exception: " + juce::String(e.what()));
    }
}

void PresetManagerTests::testPresetScanning()
{
    auto apvts = createTestAPVTS();
    PresetManager presetManager(*apvts);
    
    // Test scanning (should find factory presets)
    presetManager.scanForPresets();
    expect(presetManager.getLastError() == PresetManager::ErrorCode::None, 
           "Preset scanning should complete without errors");
    
    // Test getting preset names
    auto presetNames = presetManager.getPresetNames();
    expect(presetNames.size() > 0, "Should find at least some presets");
    
    // Test getting categories
    auto categories = presetManager.getCategories();
    expect(categories.size() > 0, "Should find at least one category");
    expect(categories.contains("Factory"), "Should contain Factory category");
    
    logMessage("✓ Preset scanning successful");
}

void PresetManagerTests::testPresetLoading()
{
    auto apvts = createTestAPVTS();
    PresetManager presetManager(*apvts);
    
    // Ensure we have some presets loaded
    presetManager.scanForPresets();
    auto presetNames = presetManager.getPresetNames();
    
    if (presetNames.size() > 0)
    {
        // Test loading first available preset
        juce::String firstPreset = presetNames[0];
        bool loaded = presetManager.loadPreset(firstPreset);
        expect(loaded, "Should be able to load existing preset: " + firstPreset);
        expect(presetManager.getCurrentPresetName() == firstPreset, 
               "Current preset name should match loaded preset");
    }
    
    // Test loading non-existent preset
    bool loadedFake = presetManager.loadPreset("NonExistentPreset123");
    expect(!loadedFake, "Should not be able to load non-existent preset");
    expect(presetManager.getLastError() == PresetManager::ErrorCode::PresetNotFound,
           "Should report PresetNotFound error for non-existent preset");
    
    logMessage("✓ Preset loading successful");
}

void PresetManagerTests::testPresetSaving()
{
    auto apvts = createTestAPVTS();
    PresetManager presetManager(*apvts);
    
    // Set some parameter values
    if (auto* rateParam = apvts->getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(0.6f);
    if (auto* depthParam = apvts->getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    
    // Test saving a preset
    juce::String testPresetName = "TestPreset_" + juce::String(juce::Random().nextInt(10000));
    bool saved = presetManager.savePreset(testPresetName, "User");
    expect(saved, "Should be able to save preset");
    expect(presetManager.getLastError() == PresetManager::ErrorCode::None,
           "Save operation should complete without errors");
    
    // Test that the preset appears in the list
    auto presetNames = presetManager.getPresetNames();
    expect(presetNames.contains(testPresetName), "Saved preset should appear in preset list");
    
    // Test loading the saved preset
    bool loaded = presetManager.loadPreset(testPresetName);
    expect(loaded, "Should be able to load saved preset");
    
    logMessage("✓ Preset saving successful");
}

void PresetManagerTests::testFolderHierarchy()
{
    auto apvts = createTestAPVTS();
    PresetManager presetManager(*apvts);
    
    presetManager.scanForPresets();
    
    // Test factory categories
    auto factoryCategories = presetManager.getFactoryCategories();
    expect(factoryCategories.size() > 0, "Should have factory categories");
    expect(factoryCategories.contains("Factory"), "Should contain root Factory category");
    
    // Test getting presets in specific folders
    auto factoryPresets = presetManager.getPresetsInFolder("Factory");
    // Don't require specific count as it depends on which presets are loaded
    
    logMessage("✓ Folder hierarchy successful");
}

void PresetManagerTests::testPresetModification()
{
    auto apvts = createTestAPVTS();
    PresetManager presetManager(*apvts);
    
    // Load a preset
    presetManager.scanForPresets();
    auto presetNames = presetManager.getPresetNames();
    
    if (presetNames.size() > 0)
    {
        presetManager.loadPreset(presetNames[0]);
        
        // Initially should not be modified
        expect(!presetManager.isPresetModified(), "Freshly loaded preset should not be modified");
        
        // Modify a parameter
        if (auto* rateParam = apvts->getParameter("lfoRate"))
        {
            rateParam->setValueNotifyingHost(0.9f);
            
            // Should now be modified
            expect(presetManager.isPresetModified(), "Preset should be marked as modified after parameter change");
            
            // Display name should include modification indicator
            juce::String displayName = presetManager.getModifiedDisplayName();
            expect(displayName.contains("*") || displayName != presetNames[0], 
                   "Modified display name should indicate modification");
        }
    }
    
    logMessage("✓ Preset modification tracking successful");
}

void PresetManagerTests::testErrorHandling()
{
    auto apvts = createTestAPVTS();
    PresetManager presetManager(*apvts);
    
    // Test saving with invalid name
    bool saved1 = presetManager.savePreset(""); // Empty name
    expect(!saved1, "Should not save preset with empty name");
    expect(presetManager.getLastError() == PresetManager::ErrorCode::InvalidPresetName,
           "Should report InvalidPresetName error");
    
    // Test saving with invalid characters
    bool saved2 = presetManager.savePreset("Invalid/Name\\*?");
    expect(!saved2, "Should not save preset with invalid characters");
    
    // Clear error and verify
    presetManager.clearError();
    expect(presetManager.getLastError() == PresetManager::ErrorCode::None,
           "Error should be cleared");
    
    logMessage("✓ Error handling successful");
}

void PresetManagerTests::testFactoryPresets()
{
    auto apvts = createTestAPVTS();
    PresetManager presetManager(*apvts);
    
    // Clear any existing factory presets
    presetManager.clearFactoryPresets();
    
    // Test loading factory presets (this requires including the Presets functionality)
    auto presetNames = presetManager.getFactoryPresetNames();
    // After clearing, should have fewer factory presets
    
    logMessage("✓ Factory presets handling successful");
}

// Helper methods
std::unique_ptr<juce::AudioProcessorValueTreeState> PresetManagerTests::createTestAPVTS()
{
    // Create a minimal APVTS for testing
    auto layout = std::make_unique<juce::AudioProcessorValueTreeState::ParameterLayout>();
    
    // Add basic parameters
    layout->add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoRate", 1), "LFO Rate",
        juce::NormalisableRange<float>(0.01f, 25.0f), 1.0f));
    
    layout->add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoDepth", 1), "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    
    layout->add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lfoWaveform", 1), "LFO Waveform",
        juce::StringArray{"Sine", "Square", "Triangle"}, 0));
    
    // Create a dummy processor for the APVTS
    struct DummyProcessor : public juce::AudioProcessor
    {
        DummyProcessor() : juce::AudioProcessor(BusesProperties()) {}
        void prepareToPlay(double, int) override {}
        void releaseResources() override {}
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        double getTailLengthSeconds() const override { return 0; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        const juce::String getName() const override { return "Test"; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return "Test"; }
        void changeProgramName(int, const juce::String&) override {}
        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}
    };
    
    static DummyProcessor dummyProcessor;
    
    return std::make_unique<juce::AudioProcessorValueTreeState>(
        dummyProcessor, nullptr, "TestParameters", std::move(*layout));
}

juce::File PresetManagerTests::createTempPresetDirectory()
{
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                      .getChildFile("TremoloViola_Test_" + juce::String(juce::Random().nextInt()));
    tempDir.createDirectory();
    return tempDir;
}

void PresetManagerTests::cleanupTempDirectory(const juce::File& dir)
{
    if (dir.exists())
        dir.deleteRecursively();
}