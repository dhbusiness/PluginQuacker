/*
  ==============================================================================

    SmokeTests.cpp
    Created: Smoke Tests for TremoloViola Plugin
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "SmokeTests.h"
#include "../QuackerVST/Source/PluginProcessor.h"

void SmokeTests::runTest()
{
    beginTest("Plugin Instantiation");
    testPluginInstantiation();
    
    beginTest("Basic Audio Processing");
    testBasicAudioProcessing();
    
    beginTest("Parameter Manipulation");
    testParameterManipulation();
    
    beginTest("Preset Loading");
    testPresetLoading();
    
    beginTest("State Restoration");
    testStateRestoration();
}

void SmokeTests::testPluginInstantiation()
{
    try
    {
        // Test basic plugin creation
        auto processor = std::make_unique<QuackerVSTAudioProcessor>();
        expect(processor != nullptr, "Plugin processor should be created successfully");
        
        // Test plugin metadata
        expect(processor->getName().isNotEmpty(), "Plugin should have a name");
        expect(processor->getTailLengthSeconds() >= 0.0, "Tail length should be non-negative");
        
        // Test channel configuration
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add(juce::AudioChannelSet::stereo());
        layout.outputBuses.add(juce::AudioChannelSet::stereo());
        expect(processor->isBusesLayoutSupported(layout), "Stereo layout should be supported");
        
        logMessage("✓ Plugin instantiation successful");
    }
    catch (const std::exception& e)
    {
        expect(false, "Plugin instantiation threw exception: " + juce::String(e.what()));
    }
    catch (...)
    {
        expect(false, "Plugin instantiation threw unknown exception");
    }
}

void SmokeTests::testBasicAudioProcessing()
{
    try
    {
        auto processor = std::make_unique<QuackerVSTAudioProcessor>();
        
        // Prepare to play
        constexpr double sampleRate = 44100.0;
        constexpr int blockSize = 512;
        processor->setPlayConfigDetails(2, 2, sampleRate, blockSize);
        processor->prepareToPlay(sampleRate, blockSize);
        
        // Create test buffer with sine wave
        juce::AudioBuffer<float> buffer(2, blockSize);
        for (int channel = 0; channel < 2; ++channel)
        {
            for (int sample = 0; sample < blockSize; ++sample)
            {
                float value = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * sample / sampleRate);
                buffer.setSample(channel, sample, value * 0.5f); // Moderate level
            }
        }
        
        // Process the buffer
        juce::MidiBuffer midiBuffer;
        processor->processBlock(buffer, midiBuffer);
        
        // Check that audio is still present (not silent)
        bool hasAudio = false;
        for (int channel = 0; channel < 2; ++channel)
        {
            for (int sample = 0; sample < blockSize; ++sample)
            {
                if (std::abs(buffer.getSample(channel, sample)) > 0.001f)
                {
                    hasAudio = true;
                    break;
                }
            }
        }
        
        expect(hasAudio, "Audio processing should produce non-silent output");
        
        // Check for NaN or inf values
        bool hasValidValues = true;
        for (int channel = 0; channel < 2; ++channel)
        {
            for (int sample = 0; sample < blockSize; ++sample)
            {
                float value = buffer.getSample(channel, sample);
                if (!std::isfinite(value))
                {
                    hasValidValues = false;
                    break;
                }
            }
        }
        
        expect(hasValidValues, "Audio output should contain finite values (no NaN/inf)");
        
        processor->releaseResources();
        logMessage("✓ Basic audio processing successful");
    }
    catch (const std::exception& e)
    {
        expect(false, "Audio processing threw exception: " + juce::String(e.what()));
    }
}

void SmokeTests::testParameterManipulation()
{
    try
    {
        auto processor = std::make_unique<QuackerVSTAudioProcessor>();
        
        // Test getting parameters
        auto& params = processor->getParameters();
        expect(params.size() > 0, "Plugin should have parameters");
        
        // Test specific parameters exist
        auto* rateParam = processor->apvts.getRawParameterValue("lfoRate");
        auto* depthParam = processor->apvts.getRawParameterValue("lfoDepth");
        auto* waveformParam = processor->apvts.getRawParameterValue("lfoWaveform");
        
        expect(rateParam != nullptr, "LFO Rate parameter should exist");
        expect(depthParam != nullptr, "LFO Depth parameter should exist");
        expect(waveformParam != nullptr, "LFO Waveform parameter should exist");
        
        // Test parameter value changes
        if (auto* rate = processor->apvts.getParameter("lfoRate"))
        {
            float originalValue = rate->getValue();
            rate->setValueNotifyingHost(0.5f);
            expect(rate->getValue() == 0.5f, "Parameter value should change when set");
            rate->setValueNotifyingHost(originalValue); // Restore
        }
        
        logMessage("✓ Parameter manipulation successful");
    }
    catch (const std::exception& e)
    {
        expect(false, "Parameter manipulation threw exception: " + juce::String(e.what()));
    }
}

void SmokeTests::testPresetLoading()
{
    try
    {
        auto processor = std::make_unique<QuackerVSTAudioProcessor>();
        
        // Test preset manager exists
        auto& presetManager = processor->getPresetManager();
        
        // Test loading a known preset (Default should always exist)
        bool loaded = presetManager.loadPreset("Default");
        expect(loaded, "Should be able to load Default preset");
        
        // Test getting preset names
        auto presetNames = presetManager.getPresetNames();
        expect(presetNames.size() > 0, "Should have at least one preset");
        expect(presetNames.contains("Default"), "Should contain Default preset");
        
        logMessage("✓ Preset loading successful");
    }
    catch (const std::exception& e)
    {
        expect(false, "Preset loading threw exception: " + juce::String(e.what()));
    }
}

void SmokeTests::testStateRestoration()
{
    try
    {
        auto processor1 = std::make_unique<QuackerVSTAudioProcessor>();
        auto processor2 = std::make_unique<QuackerVSTAudioProcessor>();
        
        // Set some parameter values in processor1
        if (auto* rateParam = processor1->apvts.getParameter("lfoRate"))
            rateParam->setValueNotifyingHost(0.75f);
        if (auto* depthParam = processor1->apvts.getParameter("lfoDepth"))
            depthParam->setValueNotifyingHost(0.25f);
        
        // Save state from processor1
        juce::MemoryBlock stateData;
        processor1->getStateInformation(stateData);
        expect(stateData.getSize() > 0, "State data should not be empty");
        
        // Restore state to processor2
        processor2->setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
        
        // Verify parameters match
        auto* rate1 = processor1->apvts.getParameter("lfoRate");
        auto* rate2 = processor2->apvts.getParameter("lfoRate");
        auto* depth1 = processor1->apvts.getParameter("lfoDepth");
        auto* depth2 = processor2->apvts.getParameter("lfoDepth");
        
        if (rate1 && rate2)
        {
            expect(std::abs(rate1->getValue() - rate2->getValue()) < 0.01f, 
                   "Rate parameter should be restored correctly");
        }
        
        if (depth1 && depth2)
        {
            expect(std::abs(depth1->getValue() - depth2->getValue()) < 0.01f, 
                   "Depth parameter should be restored correctly");
        }
        
        logMessage("✓ State restoration successful");
    }
    catch (const std::exception& e)
    {
        expect(false, "State restoration threw exception: " + juce::String(e.what()));
    }
}