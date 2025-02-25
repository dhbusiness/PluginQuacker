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
        SlowGear
    };

    TremoloLFO();
    
    void setSampleRate(double newSampleRate);
    void setRate(float newRate);
    void setDepth(float newDepth);
    void setWaveform(Waveform newWaveform);
    void setSyncMode(bool shouldSync, double division = 1.0);
    void setPhaseOffset(float offsetDegrees);
    void setBeatPosition(double newBeatPosition);
    void resetPhase();
    float getNextSample();
    bool isWaitingForReset() const;
    void updateActiveState(bool isActive, bool isPlaying);
    void setBPM(double bpm);

    // Convert BPM and note division to equivalent frequency
    static double bpmToFrequency(double bpm, double noteDivisionIndex) {
        // Convert from division index to actual multiplier
        double divisionMultiplier;
        
        // Based on your code, it appears these values would work:
        // 0.0 -> 0.25 (whole note - one cycle every 4 beats)
        // 1.0 -> 0.5 (half note - one cycle every 2 beats)
        // 2.0 -> 1.0 (quarter note - one cycle per beat)
        // 3.0 -> 2.0 (eighth note - two cycles per beat)
        // 4.0 -> 4.0 (sixteenth note - four cycles per beat)
        // 5.0 -> 8.0 (thirty-second note - eight cycles per beat)
        
        switch (int(noteDivisionIndex)) {
            case 0: divisionMultiplier = 0.25; break; // 1/1 (whole note)
            case 1: divisionMultiplier = 0.5; break;  // 1/2 (half note)
            case 2: divisionMultiplier = 1.0; break;  // 1/4 (quarter note)
            case 3: divisionMultiplier = 2.0; break;  // 1/8 (eighth note)
            case 4: divisionMultiplier = 4.0; break;  // 1/16 (sixteenth note)
            case 5: divisionMultiplier = 8.0; break;  // 1/32 (thirty-second note)
            default: divisionMultiplier = 1.0; break; // Default to quarter note
        }
        
        // First calculate cycles per minute
        double cyclesPerMinute = (bpm * divisionMultiplier);
        
        // Convert to Hz (cycles per second)
        double frequencyHz = cyclesPerMinute / 60.0;
        
        // Apply a scaling factor to keep higher divisions within a musical range
        // This helps prevent excessive rates at high BPMs with small divisions
        if (divisionMultiplier > 2.0) { // For divisions faster than 1/8 note
            frequencyHz *= 0.75; // Reduce the frequency to keep it more musical
        }
        
        return frequencyHz;
    }

    // Get the current effective frequency, whether synced or not
    double getCurrentEffectiveRate() const {
        if (syncedToHost) {
            return bpmToFrequency(currentBPM, noteDivision);
        }
        return rate;
    };
    
    bool isSynced() const { return syncedToHost; };
    double getCurrentDivision() const { return noteDivision; };
    
    // Get the remembered manual rate
    float getLastManualRate() const { return lastManualRate; };
    
    // Update the remembered manual rate
    void storeManualRate(float manualRate) { lastManualRate = manualRate; };
    
    // Convert a normalized (0-1) value to an exponential rate
    static float normalizedToRate(float normalizedValue) {
        // Map 0-1 to our frequency range (0.01 Hz to 25 Hz) exponentially
        const float minRate = 0.01f;
        const float maxRate = 25.0f;
        
        // Use an exponential curve with base 2 for musical scaling
        float expValue = std::pow(2.0f, normalizedValue * std::log2(maxRate / minRate));
        return minRate * expValue;
    };
    
    // Convert a rate back to a normalized (0-1) value
    static float rateToNormalized(float rate) {
        const float minRate = 0.01f;
        const float maxRate = 25.0f;
        
        // Inverse of the above function
        return std::log2(rate / minRate) / std::log2(maxRate / minRate);
    };

    void setWaveshapeParameters(float rate, float depth, int waveform, bool enabled) {
        waveshaper.setRate(rate);
        waveshaper.setDepth(depth);
        waveshaper.setWaveform(static_cast<WaveshapeLFO::Waveform>(waveform));
        waveshaper.setEnabled(enabled);
    }
    
private:
    double getPhaseNormalized() const;
    double getPhaseWithOffset() const;
    float generateOversampledOutput();
    float handleResetTransition();
    void updateOversamplingFactor();
    float calculateCurrentValue(double outputPhase, double smoothedPhase);

    // Member variables
    double phase;
    double accumulatedPhase;
    float rate;
    float depth;
    Waveform waveform;
    double sampleRate;
    double phaseOffset;
    float currentRate;
    float rateSmoothing;
    
    juce::SmoothedValue<float> smoothedDepth;
    juce::SmoothedValue<float> smoothedRate;
    juce::SmoothedValue<float> phaseSmoothing;
    
    bool syncedToHost;
    double beatPosition;
    double lastBeatPosition;
    double noteDivision;
    
    bool waitingForReset;
    bool wasActive;
    bool inResetTransition;
    
    float resetTransitionPhase;
    float lastOutputValue;
    const float resetTransitionTime;
    float resetTransitionIncrement;

    int oversamplingFactor;
    std::vector<float> oversampledBuffer;
    
    double currentBPM;
    
    // Store the last manual rate setting before sync was enabled
    float lastManualRate = 1.0f;
    
    WaveshapeLFO waveshaper;
    float applyWaveshaping(float input);
    
    // Cache for complex waveform calculations
    struct WaveformCache {
        double lastPhase = -1.0;
        double cachedValue = 0.0;
    };
    WaveformCache waveformCache;
};
