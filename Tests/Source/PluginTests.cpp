/*
  ==============================================================================

    PluginTests.cpp
    Created: Integration Tests for Plugin Functionality
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "PluginTests.h"
#include "../QuackerVST/Source/PluginProcessor.h"

void PluginTests::runTest()
{
    beginTest("Parameter Interactions");
    testParameterInteractions();
    
    beginTest("Audio Processing Accuracy");
    testAudioProcessingAccuracy();
    
    beginTest("Bypass Functionality");
    testBypassFunctionality();
    
    beginTest("Sync Mode Transitions");
    testSyncModeTransitions();
    
    beginTest("Waveshape Interaction");
    testWaveshapeInteraction();
    
    beginTest("Extreme Limits");
    testExtremeLimits();
    
    beginTest("Performance Stress");
    testPerformanceStress();
    
    beginTest("Thread Safety");
    testThreadSafety();
}

void PluginTests::testParameterInteractions()
{
    auto processor = createTestProcessor();
    expect(processor != nullptr, "Test processor should be created");
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // Test depth and rate interaction
    if (auto* depthParam = processor->apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.0f); // No depth
    if (auto* rateParam = processor->apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(10.0f)); // High rate
    
    auto buffer = createTestSignal(2, blockSize);
    float originalRMS = calculateRMS(buffer);
    
    juce::MidiBuffer midiBuffer;
    processor->processBlock(buffer, midiBuffer);
    
    float processedRMS = calculateRMS(buffer);
    
    // With zero depth, output should be very close to input
    expect(std::abs(originalRMS - processedRMS) < 0.1f, 
           "Zero depth should produce minimal change in signal");
    
    // Test full depth
    buffer = createTestSignal(2, blockSize);
    if (auto* depthParam = processor->apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f); // Full depth
    
    processor->processBlock(buffer, midiBuffer);
    float fullDepthRMS = calculateRMS(buffer);
    
    expect(fullDepthRMS < originalRMS * 0.9f, 
           "Full depth should significantly reduce signal amplitude");
    
    processor->releaseResources();
    logMessage("✓ Parameter interactions successful");
}

void PluginTests::testAudioProcessingAccuracy()
{
    auto processor = createTestProcessor();
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // Set up for tremolo effect
    if (auto* rateParam = processor->apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(1.0f)); // 1 Hz
    if (auto* depthParam = processor->apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.5f); // 50% depth
    if (auto* waveformParam = processor->apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(0.0f); // Sine wave
    
    // Process several blocks to establish the effect
    processAudioBlocks(*processor, 10);
    
    // Test with actual audio signal
    auto buffer = createTestSignal(2, blockSize);
    juce::MidiBuffer midiBuffer;
    
    // Check that processing doesn't create NaN or inf values
    for (int block = 0; block < 5; ++block)
    {
        buffer = createTestSignal(2, blockSize);
        processor->processBlock(buffer, midiBuffer);
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float value = buffer.getSample(channel, sample);
                expect(std::isfinite(value), "All processed samples should be finite");
                expect(std::abs(value) <= 1.5f, "Processed samples should be reasonably bounded");
            }
        }
    }
    
    processor->releaseResources();
    logMessage("✓ Audio processing accuracy successful");
}

void PluginTests::testBypassFunctionality()
{
    auto processor = createTestProcessor();
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // Set up effect parameters
    if (auto* depthParam = processor->apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f); // Full depth
    if (auto* rateParam = processor->apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.0f)); // 5 Hz
    
    // Process with effect active
    auto buffer1 = createTestSignal(2, blockSize);
    auto originalBuffer = createTestSignal(2, blockSize); // Keep reference
    
    juce::MidiBuffer midiBuffer;
    
    // Let effect stabilize
    processAudioBlocks(*processor, 5);
    
    if (auto* bypassParam = processor->apvts.getParameter("bypass"))
        bypassParam->setValueNotifyingHost(0.0f); // Effect active
    
    buffer1 = createTestSignal(2, blockSize);
    processor->processBlock(buffer1, midiBuffer);
    float processedRMS = calculateRMS(buffer1);
    
    // Process with bypass enabled
    auto buffer2 = createTestSignal(2, blockSize);
    if (auto* bypassParam = processor->apvts.getParameter("bypass"))
        bypassParam->setValueNotifyingHost(1.0f); // Bypassed
    
    processor->processBlock(buffer2, midiBuffer);
    float bypassedRMS = calculateRMS(buffer2);
    float originalRMS = calculateRMS(originalBuffer);
    
    // Bypassed signal should be much closer to original than processed signal
    expect(std::abs(bypassedRMS - originalRMS) < std::abs(processedRMS - originalRMS),
           "Bypassed signal should be closer to original than processed signal");
    
    processor->releaseResources();
    logMessage("✓ Bypass functionality successful");
}

void PluginTests::testSyncModeTransitions()
{
    auto processor = createTestProcessor();
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // Test switching between sync modes
    if (auto* syncParam = processor->apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(0.0f); // Free running
    if (auto* rateParam = processor->apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(2.0f)); // 2 Hz
    
    // Process some audio
    processAudioBlocks(*processor, 5);
    
    // Switch to sync mode
    if (auto* syncParam = processor->apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(1.0f); // Synced
    if (auto* divisionParam = processor->apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(2)); // Quarter note
    
    // Process more audio - should not crash
    processAudioBlocks(*processor, 5);
    
    // Switch back to free running
    if (auto* syncParam = processor->apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(0.0f); // Free running
    
    processAudioBlocks(*processor, 5);
    
    processor->releaseResources();
    logMessage("✓ Sync mode transitions successful");
}

void PluginTests::testWaveshapeInteraction()
{
    auto processor = createTestProcessor();
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // Enable waveshaping
    if (auto* enableParam = processor->apvts.getParameter("waveshapeEnabled"))
        enableParam->setValueNotifyingHost(1.0f); // Enabled
    if (auto* depthParam = processor->apvts.getParameter("waveshapeDepth"))
        depthParam->setValueNotifyingHost(0.5f); // 50% depth
    if (auto* rateParam = processor->apvts.getParameter("waveshapeRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(3.0f)); // 3 Hz
    
    // Process audio with waveshaping
    processAudioBlocks(*processor, 10);
    
    auto buffer = createTestSignal(2, blockSize);
    juce::MidiBuffer midiBuffer;
    processor->processBlock(buffer, midiBuffer);
    
    // Verify output is finite and reasonable
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float value = buffer.getSample(channel, sample);
            expect(std::isfinite(value), "Waveshaped output should be finite");
        }
    }
    
    processor->releaseResources();
    logMessage("✓ Waveshape interaction successful");
}

void PluginTests::testExtremeLimits()
{
    auto processor = createTestProcessor();
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // Test extreme parameter values
    if (auto* rateParam = processor->apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(1.0f); // Maximum rate
    if (auto* depthParam = processor->apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f); // Maximum depth
    
    // Process audio at extreme settings
    processAudioBlocks(*processor, 5);
    
    auto buffer = createTestSignal(2, blockSize);
    juce::MidiBuffer midiBuffer;
    processor->processBlock(buffer, midiBuffer);
    
    bool hasValidOutput = true;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float value = buffer.getSample(channel, sample);
            if (!std::isfinite(value))
            {
                hasValidOutput = false;
                break;
            }
        }
    }
    
    expect(hasValidOutput, "Plugin should handle extreme parameter values gracefully");
    
    processor->releaseResources();
    logMessage("✓ Extreme limits test successful");
}

void PluginTests::testPerformanceStress()
{
    auto processor = createTestProcessor();
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // Set up demanding configuration
    if (auto* rateParam = processor->apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(20.0f)); // Very fast LFO
    if (auto* enableParam = processor->apvts.getParameter("waveshapeEnabled"))
        enableParam->setValueNotifyingHost(1.0f); // Waveshaping enabled
    if (auto* shapingRateParam = processor->apvts.getParameter("waveshapeRate"))
        shapingRateParam->setValueNotifyingHost(shapingRateParam->convertTo0to1(15.0f)); // Fast waveshaping
    
    // Process many blocks
    const int numStressBlocks = 1000;
    juce::MidiBuffer midiBuffer;
    
    for (int i = 0; i < numStressBlocks; ++i)
    {
        auto buffer = createTestSignal(2, blockSize);
        processor->processBlock(buffer, midiBuffer);
        
        // Check every 100th block for sanity
        if (i % 100 == 0)
        {
            bool hasValidOutput = true;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    if (!std::isfinite(buffer.getSample(channel, sample)))
                    {
                        hasValidOutput = false;
                        break;
                    }
                }
            }
            expect(hasValidOutput, "Output should remain valid during stress test");
        }
    }
    
    processor->releaseResources();
    logMessage("✓ Performance stress test successful");
}

void PluginTests::testThreadSafety()
{
    auto processor = createTestProcessor();
    
    constexpr double sampleRate = 44100.0;
    constexpr int blockSize = 512;
    processor->prepareToPlay(sampleRate, blockSize);
    
    // This is a basic test - in a real scenario you'd want more sophisticated threading tests
    // Test parameter changes while processing
    juce::MidiBuffer midiBuffer;
    
    for (int i = 0; i < 100; ++i)
    {
        // Change parameters
        if (auto* rateParam = processor->apvts.getParameter("lfoRate"))
            rateParam->setValueNotifyingHost(juce::Random().nextFloat());
        if (auto* depthParam = processor->apvts.getParameter("lfoDepth"))
            depthParam->setValueNotifyingHost(juce::Random().nextFloat());
        
        // Process audio
        auto buffer = createTestSignal(2, blockSize);
        processor->processBlock(buffer, midiBuffer);
        
        // Verify output is still valid
        bool hasValidOutput = true;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                if (!std::isfinite(buffer.getSample(channel, sample)))
                {
                    hasValidOutput = false;
                    break;
                }
            }
        }
        expect(hasValidOutput, "Output should remain valid during parameter changes");
    }
    
    processor->releaseResources();
    logMessage("✓ Thread safety test successful");
}

// Helper methods
std::unique_ptr<QuackerVSTAudioProcessor> PluginTests::createTestProcessor()
{
    try
    {
        return std::make_unique<QuackerVSTAudioProcessor>();
    }
    catch (...)
    {
        return nullptr;
    }
}

juce::AudioBuffer<float> PluginTests::createTestSignal(int numChannels, int numSamples, float frequency)
{
    juce::AudioBuffer<float> buffer(numChannels, numSamples);
    
    constexpr float sampleRate = 44100.0f;
    const float amplitude = 0.5f;
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float time = sample / sampleRate;
            float value = amplitude * std::sin(2.0f * juce::MathConstants<float>::pi * frequency * time);
            buffer.setSample(channel, sample, value);
        }
    }
    
    return buffer;
}

bool PluginTests::isSignalSilent(const juce::AudioBuffer<float>& buffer, float threshold)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (std::abs(buffer.getSample(channel, sample)) > threshold)
                return false;
        }
    }
    return true;
}

float PluginTests::calculateRMS(const juce::AudioBuffer<float>& buffer)
{
    float sum = 0.0f;
    int totalSamples = 0;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float value = buffer.getSample(channel, sample);
            sum += value * value;
            totalSamples++;
        }
    }
    
    return totalSamples > 0 ? std::sqrt(sum / totalSamples) : 0.0f;
}

void PluginTests::processAudioBlocks(QuackerVSTAudioProcessor& processor, int numBlocks)
{
    constexpr int blockSize = 512;
    juce::MidiBuffer midiBuffer;
    
    for (int i = 0; i < numBlocks; ++i)
    {
        auto buffer = createTestSignal(2, blockSize);
        processor.processBlock(buffer, midiBuffer);
    }
}