/*
  ==============================================================================

    PresetManagerTests.h
    Created: Unit Tests for PresetManager Class
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * Unit tests for the PresetManager class, testing preset loading, saving,
 * scanning, and folder hierarchy functionality.
 */
class PresetManagerTests : public juce::UnitTest
{
public:
    PresetManagerTests() : juce::UnitTest("PresetManager Tests", "Unit") {}

    void runTest() override;

private:
    void testInitialization();
    void testPresetScanning();
    void testPresetLoading();
    void testPresetSaving();
    void testFolderHierarchy();
    void testPresetModification();
    void testErrorHandling();
    void testFactoryPresets();
    
    // Helper methods
    std::unique_ptr<juce::AudioProcessorValueTreeState> createTestAPVTS();
    juce::File createTempPresetDirectory();
    void cleanupTempDirectory(const juce::File& dir);
};