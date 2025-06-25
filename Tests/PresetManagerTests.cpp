/*
  ==============================================================================

    PresetManagerTests.cpp
    Unit tests for PresetManager class
    
  ==============================================================================
*/

#include "QuackerVSTTests.h"

class PresetManagerTests : public QuackerTestBase {
public:
    PresetManagerTests() : QuackerTestBase("PresetManager Tests") {}
    
    void runTest() override {
        beginTest("PresetManager Tests");
        
        DiagnosticLogger::getInstance().startLogging("preset_manager_test_log.txt");
        
        runSubTest("Initialization Test", [this]() { testInitialization(); });
        runSubTest("Save and Load Test", [this]() { testSaveAndLoad(); });
        runSubTest("Factory Presets Test", [this]() { testFactoryPresets(); });
        runSubTest("Preset Validation Test", [this]() { testPresetValidation(); });
        runSubTest("Category Management Test", [this]() { testCategoryManagement(); });
        runSubTest("File Operations Test", [this]() { testFileOperations(); });
        runSubTest("Preset Modification Detection", [this]() { testModificationDetection(); });
        runSubTest("Thread Safety Test", [this]() { testThreadSafety(); });
        runSubTest("Error Handling Test", [this]() { testErrorHandling(); });
        runSubTest("Performance Test", [this]() { testPerformance(); });
        
        DiagnosticLogger::getInstance().stopLogging();
    }
    
private:
    void testInitialization() {
        // Create a test APVTS
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        
        expect(presetManager.getCurrentPresetDirectory().exists(), 
               "Preset directory should exist after initialization");
        
        expect(presetManager.getLastError() == PresetManager::ErrorCode::None,
               "Should initialize without errors");
        
        expect(presetManager.getCurrentPresetName() == "Default",
               "Should start with Default preset");
        
        DIAG_LOG("PresetManager", "Initialization successful");
    }
    
    void testSaveAndLoad() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        auto& apvts = processor.apvts;
        
        // Set specific parameter values
        if (auto* rateParam = apvts.getParameter("lfoRate")) {
            rateParam->setValueNotifyingHost(rateParam->convertTo0to1(7.5f));
        }
        if (auto* depthParam = apvts.getParameter("lfoDepth")) {
            depthParam->setValueNotifyingHost(0.75f);
        }
        if (auto* waveformParam = apvts.getParameter("lfoWaveform")) {
            waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(3));
        }
        
        // Save preset
        juce::String testPresetName = "UnitTest_" + juce::String(juce::Random::getSystemRandom().nextInt());
        bool saved = presetManager.savePreset(testPresetName, "User");
        expect(saved, "Preset should save successfully");
        
        // Load a different preset to change state
        presetManager.loadPreset("Default");
        
        // Load our test preset
        bool loaded = presetManager.loadPreset(testPresetName);
        expect(loaded, "Preset should load successfully");
        
        // Verify parameters were restored
        auto* rateParam = apvts.getRawParameterValue("lfoRate");
        auto* depthParam = apvts.getRawParameterValue("lfoDepth");
        auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
        
        expectWithinAbsoluteError(rateParam->load(), 7.5f, 0.1f, 
                                "Rate should be restored");
        expectWithinAbsoluteError(depthParam->load(), 0.75f, 0.01f, 
                                "Depth should be restored");
        expect(static_cast<int>(waveformParam->load()) == 3, 
               "Waveform should be restored");
        
        // Clean up test preset
        auto presetFile = presetManager.getCurrentPresetDirectory()
                            .getChildFile("User")
                            .getChildFile(testPresetName + ".xml");
        if (presetFile.exists()) {
            presetFile.deleteFile();
        }
        
        DIAG_LOG("PresetManager", "Save and load test complete");
    }
    
    void testFactoryPresets() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        
        // Load factory presets
        processor.loadFactoryPresets();
        
        auto factoryPresets = presetManager.getFactoryPresetNames();
        expect(factoryPresets.size() > 0, "Should have factory presets");
        
        // Test loading each factory preset
        int successCount = 0;
        for (const auto& presetName : factoryPresets) {
            bool loaded = presetManager.loadPreset(presetName);
            if (loaded) {
                successCount++;
                
                // Verify preset is marked as factory
                auto category = presetManager.getPresetCategory(presetName);
                expect(category.startsWith("Factory"), 
                       presetName + " should be in Factory category");
            }
        }
        
        expect(successCount == factoryPresets.size(), 
               "All factory presets should load successfully");
        
        DIAG_LOG("PresetManager", "Loaded " + juce::String(successCount) + " factory presets");
    }
    
    void testPresetValidation() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        
        // Test invalid preset names
        const juce::String invalidNames[] = {
            "",                    // Empty
            "Con",                // Reserved name on Windows
            "preset/with/slash",  // Invalid character
            "preset:with:colon",  // Invalid character
            "preset*with*star",   // Invalid character
            juce::String::repeatedString("x", 200) // Too long
        };
        
        for (const auto& invalidName : invalidNames) {
            bool saved = presetManager.savePreset(invalidName, "User");
            // Should either fail or sanitize the name
            if (saved) {
                // If it saved, the name should have been sanitized
                auto presets = presetManager.getUserPresetNames();
                bool foundExact = false;
                for (const auto& preset : presets) {
                    if (preset == invalidName) {
                        foundExact = true;
                        break;
                    }
                }
                expect(!foundExact, "Invalid name should be sanitized");
            }
        }
        
        // Test valid preset names
        const juce::String validNames[] = {
            "My Preset",
            "Preset_123",
            "Cool-Sound",
            "Test (v2)",
            "2025 Preset"
        };
        
        for (const auto& validName : validNames) {
            bool saved = presetManager.savePreset(validName, "User");
            expect(saved, validName + " should be a valid preset name");
            
            // Clean up
            auto presetFile = presetManager.getCurrentPresetDirectory()
                                .getChildFile("User")
                                .getChildFile(validName.replace(" ", "_") + ".xml");
            if (presetFile.exists()) {
                presetFile.deleteFile();
            }
        }
        
        DIAG_LOG("PresetManager", "Preset validation test complete");
    }
    
    void testCategoryManagement() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        
        // Test creating presets in different categories
        const juce::String testCategories[] = {
            "User",
            "User/MyCategory",
            "User/MyCategory/SubCategory"
        };
        
        for (const auto& category : testCategories) {
            juce::String presetName = "TestPreset_" + category.replace("/", "_");
            bool saved = presetManager.savePreset(presetName, category);
            expect(saved, "Should save preset in category: " + category);
            
            // Verify category
            auto savedCategory = presetManager.getPresetCategory(presetName);
            expect(savedCategory == category, "Category should match");
        }
        
        // Test category listing
        auto categories = presetManager.getCategories();
        expect(categories.contains("User"), "Should have User category");
        
        // Test factory categories
        auto factoryCategories = presetManager.getFactoryCategories();
        expect(factoryCategories.contains("Factory"), "Should have Factory category");
        
        DIAG_LOG("PresetManager", "Category management test complete");
    }
    
    void testFileOperations() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        
        // Test preset directory operations
        auto presetDir = presetManager.getCurrentPresetDirectory();
        expect(presetDir.exists(), "Preset directory should exist");
        expect(presetDir.isDirectory(), "Should be a directory");
        expect(presetDir.hasWriteAccess(), "Should have write access");
        
        // Test scanning for presets
        presetManager.scanForPresets();
        
        auto allPresets = presetManager.getPresetNames();
        expect(allPresets.size() > 0, "Should find some presets after scanning");
        
        // Test preset file format
        juce::String testPreset = "FileFormatTest";
        presetManager.savePreset(testPreset, "User");
        
        auto presetFile = presetDir.getChildFile("User").getChildFile(testPreset + ".xml");
        if (presetFile.exists()) {
            // Verify it's valid XML
            auto xml = juce::XmlDocument::parse(presetFile);
            expect(xml != nullptr, "Preset file should be valid XML");
            
            if (xml) {
                expect(xml->hasAttribute("name"), "Should have name attribute");
                expect(xml->hasAttribute("category"), "Should have category attribute");
                expect(xml->hasAttribute("dateCreated"), "Should have dateCreated attribute");
            }
            
            presetFile.deleteFile();
        }
        
        DIAG_LOG("PresetManager", "File operations test complete");
    }
    
    void testModificationDetection() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        auto& apvts = processor.apvts;
        
        // Load a preset
        presetManager.loadPreset("Default");
        
        // Should not be modified initially
        expect(!presetManager.isPresetModified(), "Preset should not be modified after loading");
        
        // Modify a parameter
        if (auto* rateParam = apvts.getParameter("lfoRate")) {
            float currentValue = apvts.getRawParameterValue("lfoRate")->load();
            rateParam->setValueNotifyingHost(rateParam->convertTo0to1(currentValue + 1.0f));
        }
        
        // Should now be modified
        expect(presetManager.isPresetModified(), "Preset should be modified after parameter change");
        
        // Save the preset
        presetManager.savePreset("ModificationTest", "User");
        
        // Should no longer be modified
        expect(!presetManager.isPresetModified(), "Preset should not be modified after saving");
        
        // Verify display name includes asterisk when modified
        if (auto* depthParam = apvts.getParameter("lfoDepth")) {
            depthParam->setValueNotifyingHost(0.123f);
        }
        
        auto displayName = presetManager.getModifiedDisplayName();
        expect(displayName.contains("*"), "Modified preset name should include asterisk");
        
        // Clean up
        auto presetFile = presetManager.getCurrentPresetDirectory()
                            .getChildFile("User")
                            .getChildFile("ModificationTest.xml");
        if (presetFile.exists()) {
            presetFile.deleteFile();
        }
        
        DIAG_LOG("PresetManager", "Modification detection test complete");
    }
    
    void testThreadSafety() {
        auto processor = std::make_shared<QuackerVSTAudioProcessor>();
        auto& presetManager = processor->getPresetManager();
        
        // Test concurrent preset operations
        testThreadSafety("Concurrent preset loading", [&presetManager]() {
            auto presets = presetManager.getPresetNames();
            if (!presets.isEmpty()) {
                int index = juce::Random::getSystemRandom().nextInt(presets.size());
                presetManager.loadPreset(presets[index]);
            }
        });
        
        // Test concurrent saving (with unique names)
        std::atomic<int> counter(0);
        testThreadSafety("Concurrent preset saving", [&presetManager, &counter]() {
            int id = counter++;
            juce::String presetName = "ThreadTest_" + juce::String(id);
            presetManager.savePreset(presetName, "User");
            
            // Clean up immediately
            auto presetFile = presetManager.getCurrentPresetDirectory()
                                .getChildFile("User")
                                .getChildFile(presetName.replace(" ", "_") + ".xml");
            if (presetFile.exists()) {
                presetFile.deleteFile();
            }
        });
        
        DIAG_LOG("PresetManager", "Thread safety test complete");
    }
    
    void testErrorHandling() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        
        // Test loading non-existent preset
        bool loaded = presetManager.loadPreset("ThisPresetDoesNotExist");
        expect(!loaded, "Should fail to load non-existent preset");
        expect(presetManager.getLastError() == PresetManager::ErrorCode::PresetNotFound,
               "Should report preset not found error");
        
        // Test saving to invalid location (if possible to simulate)
        // This is platform-dependent and might not be easily testable
        
        // Test clearing errors
        presetManager.clearError();
        expect(presetManager.getLastError() == PresetManager::ErrorCode::None,
               "Error should be cleared");
        
        DIAG_LOG("PresetManager", "Error handling test complete");
    }
    
    void testPerformance() {
        QuackerVSTAudioProcessor processor;
        auto& presetManager = processor.getPresetManager();
        
        // Test preset scanning performance
        {
            PerformanceTimer timer("Preset scanning");
            presetManager.scanForPresets();
        }
        
        // Test bulk preset loading
        auto presets = presetManager.getPresetNames();
        if (!presets.isEmpty()) {
            PerformanceTimer timer("Load 10 presets");
            
            for (int i = 0; i < juce::jmin(10, presets.size()); ++i) {
                presetManager.loadPreset(presets[i]);
            }
        }
        
        // Test preset saving performance
        {
            PerformanceTimer timer("Save preset");
            presetManager.savePreset("PerformanceTest", "User");
            
            // Clean up
            auto presetFile = presetManager.getCurrentPresetDirectory()
                                .getChildFile("User")
                                .getChildFile("PerformanceTest.xml");
            if (presetFile.exists()) {
                presetFile.deleteFile();
            }
        }
        
        DIAG_LOG("PresetManager", "Performance test complete");
    }
};

// Register the test
static PresetManagerTests presetManagerTests;