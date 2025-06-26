/*
  ==============================================================================

    TremoloLFOTests.cpp
    Created: Unit Tests for TremoloLFO Class
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "TremoloLFOTests.h"
#include "../QuackerVST/Source/TremoloLFO.h"

void TremoloLFOTests::runTest()
{
    beginTest("Initialization");
    testInitialization();
    
    beginTest("Parameter Validation");
    testParameterValidation();
    
    beginTest("Waveform Generation");
    testWaveformGeneration();
    
    beginTest("Sample Rate Handling");
    testSampleRateHandling();
    
    beginTest("BPM Synchronization");
    testBPMSyncronization();
    
    beginTest("Phase Offset");
    testPhaseOffset();
    
    beginTest("Error Handling");
    testErrorHandling();
    
    beginTest("Waveshaping");
    testWaveshaping();
    
    beginTest("Active State Handling");
    testActiveStateHandling();
}

void TremoloLFOTests::testInitialization()
{
    TremoloLFO lfo;
    
    // Test that LFO initializes without errors
    auto result = lfo.setSampleRate(44100.0);
    expect(result == TremoloLFO::ErrorCode::None, "LFO should initialize with valid sample rate");
    
    // Test default values produce finite output
    float output = lfo.getNextSample();
    expect(std::isfinite(output), "LFO should produce finite output by default");
    expect(output >= 0.0f && output <= 1.0f, "LFO output should be in range [0, 1]");
    
    logMessage("✓ LFO initialization successful");
}

void TremoloLFOTests::testParameterValidation()
{
    TremoloLFO lfo;
    lfo.setSampleRate(44100.0);
    
    // Test valid rate values
    expect(lfo.setRate(1.0f) == TremoloLFO::ErrorCode::None, "Valid rate should be accepted");
    expect(lfo.setRate(0.01f) == TremoloLFO::ErrorCode::None, "Minimum rate should be accepted");
    expect(lfo.setRate(25.0f) == TremoloLFO::ErrorCode::None, "Maximum rate should be accepted");
    
    // Test valid depth values
    expect(lfo.setDepth(0.5f) == TremoloLFO::ErrorCode::None, "Valid depth should be accepted");
    expect(lfo.setDepth(0.0f) == TremoloLFO::ErrorCode::None, "Minimum depth should be accepted");
    expect(lfo.setDepth(1.0f) == TremoloLFO::ErrorCode::None, "Maximum depth should be accepted");
    
    // Test valid waveforms
    expect(lfo.setWaveform(TremoloLFO::Sine) == TremoloLFO::ErrorCode::None, "Sine waveform should be accepted");
    expect(lfo.setWaveform(TremoloLFO::Square) == TremoloLFO::ErrorCode::None, "Square waveform should be accepted");
    expect(lfo.setWaveform(TremoloLFO::Triangle) == TremoloLFO::ErrorCode::None, "Triangle waveform should be accepted");
    
    // Test valid BPM values
    expect(lfo.setBPM(120.0) == TremoloLFO::ErrorCode::None, "Valid BPM should be accepted");
    expect(lfo.setBPM(60.0) == TremoloLFO::ErrorCode::None, "Lower BPM should be accepted");
    expect(lfo.setBPM(180.0) == TremoloLFO::ErrorCode::None, "Higher BPM should be accepted");
    
    logMessage("✓ Parameter validation successful");
}

void TremoloLFOTests::testWaveformGeneration()
{
    TremoloLFO lfo;
    lfo.setSampleRate(44100.0);
    lfo.setRate(1.0f); // 1 Hz
    lfo.setDepth(1.0f); // Full depth
    
    // Test sine wave generation
    lfo.setWaveform(TremoloLFO::Sine);
    lfo.resetPhase();
    
    std::vector<float> samples;
    const int numSamples = 44100; // One second at 44.1kHz
    samples.reserve(numSamples);
    
    for (int i = 0; i < numSamples; ++i)
    {
        samples.push_back(lfo.getNextSample());
    }
    
    // Verify sine wave characteristics
    expect(isSineWave(samples, 1.0f, 44100.0f), "LFO should generate a recognizable sine wave");
    
    // Test square wave
    lfo.setWaveform(TremoloLFO::Square);
    lfo.resetPhase();
    
    // Generate a few samples and check they're close to 0 or 1 (for square wave)
    bool hasLowValues = false;
    bool hasHighValues = false;
    
    for (int i = 0; i < 100; ++i)
    {
        float sample = lfo.getNextSample();
        if (sample < 0.2f) hasLowValues = true;
        if (sample > 0.8f) hasHighValues = true;
    }
    
    expect(hasLowValues && hasHighValues, "Square wave should have both low and high values");
    
    logMessage("✓ Waveform generation successful");
}

void TremoloLFOTests::testSampleRateHandling()
{
    TremoloLFO lfo;
    
    // Test various sample rates
    expect(lfo.setSampleRate(44100.0) == TremoloLFO::ErrorCode::None, "44.1kHz should be valid");
    expect(lfo.setSampleRate(48000.0) == TremoloLFO::ErrorCode::None, "48kHz should be valid");
    expect(lfo.setSampleRate(96000.0) == TremoloLFO::ErrorCode::None, "96kHz should be valid");
    
    // Test that output remains stable after sample rate changes
    lfo.setRate(1.0f);
    lfo.setDepth(0.5f);
    
    float output1 = lfo.getNextSample();
    lfo.setSampleRate(48000.0);
    float output2 = lfo.getNextSample();
    
    expect(std::isfinite(output1) && std::isfinite(output2), "Outputs should remain finite after sample rate change");
    
    logMessage("✓ Sample rate handling successful");
}

void TremoloLFOTests::testBPMSyncronization()
{
    TremoloLFO lfo;
    lfo.setSampleRate(44100.0);
    
    // Test BPM to frequency conversion
    double freq1 = TremoloLFO::bpmToFrequency(120.0, 1.0); // 120 BPM, quarter note
    expect(freq1 > 0.0, "BPM conversion should produce positive frequency");
    
    double freq2 = TremoloLFO::bpmToFrequency(120.0, 2.0); // 120 BPM, eighth note
    expect(freq2 > freq1, "Higher division should produce higher frequency");
    
    // Test sync mode
    lfo.setBPM(120.0);
    auto result = lfo.setSyncMode(true, 1.0);
    expect(result == TremoloLFO::ErrorCode::None, "Sync mode should be settable");
    expect(lfo.isSynced(), "LFO should report as synced");
    
    logMessage("✓ BPM synchronization successful");
}

void TremoloLFOTests::testPhaseOffset()
{
    TremoloLFO lfo;
    lfo.setSampleRate(44100.0);
    lfo.setWaveform(TremoloLFO::Sine);
    lfo.setRate(1.0f);
    lfo.setDepth(1.0f);
    
    // Test phase offset effects
    lfo.setPhaseOffset(0.0f);
    lfo.resetPhase();
    float sample1 = lfo.getNextSample();
    
    lfo.setPhaseOffset(180.0f); // 180 degree offset
    lfo.resetPhase();
    float sample2 = lfo.getNextSample();
    
    // With 180 degree offset, the values should be significantly different
    expect(std::abs(sample1 - sample2) > 0.1f, "180 degree phase offset should produce different output");
    
    logMessage("✓ Phase offset successful");
}

void TremoloLFOTests::testErrorHandling()
{
    TremoloLFO lfo;
    
    // Test invalid sample rates
    auto result1 = lfo.setSampleRate(-1.0);
    expect(result1 != TremoloLFO::ErrorCode::None, "Negative sample rate should be rejected");
    
    auto result2 = lfo.setSampleRate(1000000.0); // Very high sample rate
    expect(result2 != TremoloLFO::ErrorCode::None, "Extremely high sample rate should be rejected");
    
    // Test invalid rates (these should be clamped, not error)
    lfo.setSampleRate(44100.0);
    lfo.setRate(-1.0f); // Should clamp
    float output = lfo.getNextSample();
    expect(std::isfinite(output), "LFO should handle clamped negative rate gracefully");
    
    logMessage("✓ Error handling successful");
}

void TremoloLFOTests::testWaveshaping()
{
    TremoloLFO lfo;
    lfo.setSampleRate(44100.0);
    lfo.setWaveform(TremoloLFO::Sine);
    lfo.setRate(1.0f);
    lfo.setDepth(1.0f);
    
    // Test without waveshaping
    lfo.setWaveshapeParameters(1.0f, 0.0f, 0, false);
    lfo.resetPhase();
    float outputWithoutShaping = lfo.getNextSample();
    
    // Test with waveshaping
    lfo.setWaveshapeParameters(2.0f, 0.5f, 0, true);
    lfo.resetPhase();
    float outputWithShaping = lfo.getNextSample();
    
    // Both outputs should be finite
    expect(std::isfinite(outputWithoutShaping), "Output without waveshaping should be finite");
    expect(std::isfinite(outputWithShaping), "Output with waveshaping should be finite");
    
    logMessage("✓ Waveshaping successful");
}

void TremoloLFOTests::testActiveStateHandling()
{
    TremoloLFO lfo;
    lfo.setSampleRate(44100.0);
    lfo.setRate(1.0f);
    lfo.setDepth(1.0f);
    
    // Test inactive state
    lfo.updateActiveState(false, false);
    float outputInactive = lfo.getNextSample();
    
    // Test active state
    lfo.updateActiveState(true, true);
    float outputActive = lfo.getNextSample();
    
    expect(std::isfinite(outputInactive), "Inactive state should produce finite output");
    expect(std::isfinite(outputActive), "Active state should produce finite output");
    
    // Test waiting for reset
    lfo.updateActiveState(false, true); // Was active, now inactive, still playing
    expect(lfo.isWaitingForReset(), "LFO should be waiting for reset when going from active to inactive while playing");
    
    logMessage("✓ Active state handling successful");
}

// Helper methods
bool TremoloLFOTests::isApproximatelyEqual(float a, float b, float tolerance)
{
    return std::abs(a - b) <= tolerance;
}

bool TremoloLFOTests::isSineWave(const std::vector<float>& samples, float expectedFreq, float sampleRate, float tolerance)
{
    if (samples.size() < 100) return false; // Need enough samples
    
    // Simple check: verify the wave oscillates between reasonable bounds
    float min = *std::min_element(samples.begin(), samples.end());
    float max = *std::max_element(samples.begin(), samples.end());
    
    // Should have reasonable amplitude variation
    if ((max - min) < 0.1f) return false;
    
    // Check for zero crossings (rough frequency estimate)
    int zeroCrossings = 0;
    for (size_t i = 1; i < samples.size(); ++i)
    {
        if ((samples[i-1] < 0.5f && samples[i] >= 0.5f) ||
            (samples[i-1] >= 0.5f && samples[i] < 0.5f))
        {
            zeroCrossings++;
        }
    }
    
    // Rough frequency calculation
    float estimatedFreq = (zeroCrossings / 2.0f) * sampleRate / samples.size();
    
    return std::abs(estimatedFreq - expectedFreq) <= tolerance;
}