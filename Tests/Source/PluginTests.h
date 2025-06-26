/*
  ==============================================================================

    PluginTests.h
    Created: Integration Tests for Plugin Functionality
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * Integration tests for the complete plugin, testing parameter interactions,
 * audio processing under various conditions, and plugin lifecycle.
 */
class PluginTests : public juce::UnitTest
{
public:
    PluginTests() : juce::UnitTest("Plugin Integration Tests", "Integration") {}

    void runTest() override;

private:
    void testParameterInteractions();
    void testAudioProcessingAccuracy();
    void testBypassFunctionality();
    void testSyncModeTransitions();
    void testWaveshapeInteraction();
    void testExtremeLimits();
    void testPerformanceStress();
    void testThreadSafety();
    
    // Helper methods
    std::unique_ptr<class QuackerVSTAudioProcessor> createTestProcessor();
    juce::AudioBuffer<float> createTestSignal(int numChannels, int numSamples, float frequency = 440.0f);
    bool isSignalSilent(const juce::AudioBuffer<float>& buffer, float threshold = 0.0001f);
    float calculateRMS(const juce::AudioBuffer<float>& buffer);
    void processAudioBlocks(class QuackerVSTAudioProcessor& processor, int numBlocks = 10);
};