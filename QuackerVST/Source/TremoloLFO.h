/*
  ==============================================================================

    TremoloLFO.h
    Created: 14 Feb 2025 12:00:03pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "WaveshapeLFO.h"

class TremoloLFO {
public:
    enum Waveform {
        Sine,
        Square,
        Triangle,
        SawtoothUp,
        SawtoothDown,
        SoftSquare,
        FenderStyle,
        WurlitzerStyle,
        VoxStyle,
        MagnatoneStyle,
        PulseDecay,
        BouncingBall,
        MultiSine,
        OpticalStyle,
        TwinPeaks,
        SmoothRandom,
        GuitarPick,
        VintageChorus,
        SlowGear,
        NumWaveforms // Add this for bounds checking
    };

    // Error codes for non-exception error handling
    enum class ErrorCode {
        None = 0,
        InvalidSampleRate,
        InvalidBPM,
        InvalidDivision,
        InvalidRate,
        InvalidDepth,
        InvalidWaveform,
        InvalidPhaseOffset,
        BufferAllocationFailed
    };

    TremoloLFO();
    ~TremoloLFO() = default;
    
    // Modified methods with error handling
    ErrorCode setSampleRate(double newSampleRate);
    ErrorCode setRate(float newRate);
    ErrorCode setDepth(float newDepth);
    ErrorCode setWaveform(Waveform newWaveform);
    ErrorCode setSyncMode(bool shouldSync, double division = 1.0);
    ErrorCode setPhaseOffset(float offsetDegrees);
    void setBeatPosition(double newBeatPosition);
    void resetPhase();
    float getNextSample() noexcept;
    bool isWaitingForReset() const noexcept { return waitingForReset; }
    void updateActiveState(bool isActive, bool isPlaying) noexcept;
    ErrorCode setBPM(double bpm);

    // Convert BPM and note division to equivalent frequency with safety
    static double bpmToFrequency(double bpm, double noteDivision) noexcept {
        // Safety checks with constants
        constexpr double MIN_BPM = 1.0;
        constexpr double MAX_BPM = 999.0;
        constexpr double DEFAULT_BPM = 120.0;
        constexpr double MIN_DIVISION = 0.0625; // 1/16 of a beat
        constexpr double MAX_DIVISION = 16.0;    // 16 beats
        
        bpm = juce::jlimit(MIN_BPM, MAX_BPM, bpm);
        noteDivision = juce::jlimit(MIN_DIVISION, MAX_DIVISION, noteDivision);
        
        // Calculate cycles per minute
        double cyclesPerMinute = bpm * noteDivision;
        
        // Convert to Hz (cycles per second)
        double frequencyHz = cyclesPerMinute / 60.0;
        
        // Apply scaling for musical range
        if (noteDivision > 2.0) {
            frequencyHz *= 0.75;
        }
        
        // Final safety limits
        return juce::jlimit(0.01, 25.0, frequencyHz);
    }

    // Get the current effective frequency, whether synced or not
    double getCurrentEffectiveRate() const noexcept {
        if (syncedToHost) {
            return bpmToFrequency(currentBPM, noteDivision);
        }
        return rate;
    }
    
    bool isSynced() const noexcept { return syncedToHost; }
    double getCurrentDivision() const noexcept { return noteDivision; }
    float getLastManualRate() const noexcept { return lastManualRate; }
    void storeManualRate(float manualRate) noexcept;
    
    // Convert a normalized (0-1) value to an exponential rate
    static float normalizedToRate(float normalizedValue) noexcept {
        normalizedValue = juce::jlimit(0.0f, 1.0f, normalizedValue);
        constexpr float minRate = 0.01f;
        constexpr float maxRate = 25.0f;
        
        float expValue = std::pow(2.0f, normalizedValue * std::log2(maxRate / minRate));
        return juce::jlimit(minRate, maxRate, minRate * expValue);
    }
    
    // Convert a rate back to a normalized (0-1) value
    static float rateToNormalized(float rate) noexcept {
        constexpr float minRate = 0.01f;
        constexpr float maxRate = 25.0f;
        
        rate = juce::jlimit(minRate, maxRate, rate);
        return std::log2(rate / minRate) / std::log2(maxRate / minRate);
    }

    ErrorCode setWaveshapeParameters(float rate, float depth, int waveform, bool enabled);
    
    // Get last error for diagnostics
    ErrorCode getLastError() const noexcept { return lastError; }
    void clearError() noexcept { lastError = ErrorCode::None; }
    
private:
    double getPhaseNormalized() const noexcept;
    double getPhaseWithOffset() const noexcept;
    float generateOversampledOutput() noexcept;
    float handleResetTransition() noexcept;
    ErrorCode updateOversamplingFactor();
    float calculateCurrentValue(double outputPhase, double smoothedPhase) noexcept;
    float applyWaveshaping(float input) noexcept;
    
    // Validate parameters
    bool validateSampleRate(double sr) const noexcept;
    bool validateRate(float r) const noexcept;
    bool validateDepth(float d) const noexcept;
    bool validateWaveform(int w) const noexcept;
    bool validateBPM(double bpm) const noexcept;
    bool validateDivision(double div) const noexcept;

    // Member variables
    double phase = 0.0;
    double accumulatedPhase = 0.0;
    float rate = 1.0f;
    float depth = 0.5f;
    Waveform waveform = Sine;
    double sampleRate = 44100.0;
    double phaseOffset = 0.0;
    float currentRate = 1.0f;
    float rateSmoothing = 0.997f;
    
    juce::SmoothedValue<float> smoothedDepth;
    juce::SmoothedValue<float> smoothedRate;
    juce::SmoothedValue<float> phaseSmoothing;
    
    bool syncedToHost = false;
    double beatPosition = 0.0;
    double lastBeatPosition = 0.0;
    double noteDivision = 1.0;
    
    bool waitingForReset = false;
    bool wasActive = false;
    bool inResetTransition = false;
    
    float resetTransitionPhase = 0.0f;
    float lastOutputValue = 0.0f;
    const float resetTransitionTime = 0.05f;
    float resetTransitionIncrement = 0.0f;

    int oversamplingFactor = 4;
    std::vector<float> oversampledBuffer;
    
    double currentBPM = 120.0;
    float lastManualRate = 1.0f;
    
    WaveshapeLFO waveshaper;
    
    // Cache for complex waveform calculations
    struct WaveformCache {
        double lastPhase = -1.0;
        double cachedValue = 0.0;
    };
    WaveformCache waveformCache;
    
    // Error handling
    mutable ErrorCode lastError = ErrorCode::None;
    
    // Constants for validation
    static constexpr double MIN_SAMPLE_RATE = 8000.0;
    static constexpr double MAX_SAMPLE_RATE = 384000.0;
    static constexpr float MIN_RATE = 0.001f;
    static constexpr float MAX_RATE = 100.0f;
    static constexpr float MIN_DEPTH = 0.0f;
    static constexpr float MAX_DEPTH = 1.0f;
    static constexpr double MIN_BPM = 1.0;
    static constexpr double MAX_BPM = 999.0;
    static constexpr double MIN_DIVISION = 0.0625;
    static constexpr double MAX_DIVISION = 16.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloLFO)
};
