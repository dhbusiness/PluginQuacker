/*
  ==============================================================================

    TestProcessorWrapper.h
    Wrapper class to expose protected methods for testing
    
  ==============================================================================
*/

#pragma once

#include "../QuackerVST/Source/PluginProcessor.h"

// Test wrapper that exposes protected methods
class TestProcessorWrapper : public QuackerVSTAudioProcessor {
public:
    TestProcessorWrapper() : QuackerVSTAudioProcessor() {}
    
    // Expose protected methods for testing
    bool testIsBusesLayoutSupported(const juce::AudioProcessor::BusesLayout& layout) const {
        return isBusesLayoutSupported(layout);
    }
    
    // Expose other protected methods if needed
    void testPrepareToPlay(double sampleRate, int samplesPerBlock) {
        prepareToPlay(sampleRate, samplesPerBlock);
    }
    
    void testReleaseResources() {
        releaseResources();
    }
    
    // Helper methods for testing
    bool testSupportsChannelConfiguration(int inputChannels, int outputChannels) const {
        juce::AudioProcessor::BusesLayout layout;
        
        if (inputChannels == 1) {
            layout.getMainInputChannelSet() = juce::AudioChannelSet::mono();
        } else if (inputChannels == 2) {
            layout.getMainInputChannelSet() = juce::AudioChannelSet::stereo();
        } else {
            // Unsupported configuration
            layout.getMainInputChannelSet() = juce::AudioChannelSet::discreteChannels(inputChannels);
        }
        
        if (outputChannels == 1) {
            layout.getMainOutputChannelSet() = juce::AudioChannelSet::mono();
        } else if (outputChannels == 2) {
            layout.getMainOutputChannelSet() = juce::AudioChannelSet::stereo();
        } else {
            // Unsupported configuration
            layout.getMainOutputChannelSet() = juce::AudioChannelSet::discreteChannels(outputChannels);
        }
        
        return testIsBusesLayoutSupported(layout);
    }
};