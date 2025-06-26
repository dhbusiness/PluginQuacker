/*
  ==============================================================================

    TremoloLFOTests.h
    Created: Unit Tests for TremoloLFO Class
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * Unit tests for the TremoloLFO class, testing core LFO functionality,
 * waveform generation, parameter validation, and edge cases.
 */
class TremoloLFOTests : public juce::UnitTest
{
public:
    TremoloLFOTests() : juce::UnitTest("TremoloLFO Tests", "Unit") {}

    void runTest() override;

private:
    void testInitialization();
    void testParameterValidation();
    void testWaveformGeneration();
    void testSampleRateHandling();
    void testBPMSyncronization();
    void testPhaseOffset();
    void testErrorHandling();
    void testWaveshaping();
    void testActiveStateHandling();
    
    // Helper methods
    bool isApproximatelyEqual(float a, float b, float tolerance = 0.001f);
    bool isSineWave(const std::vector<float>& samples, float expectedFreq, float sampleRate, float tolerance = 0.1f);
};