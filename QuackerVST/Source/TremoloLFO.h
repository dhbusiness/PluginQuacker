/*
  ==============================================================================

    TremoloLFO.h
    Created: 10 Feb 2025 2:45:13pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class TremoloLFO
{
public:
    enum Waveform {
        Sine,
        Square,
        Triangle,
        SawtoothUp,
        SawtoothDown,
        SoftSquare,
        FenderStyle,
        WurlitzerStyle
    };
    
    TremoloLFO()
        : phase(0.0)
        , rate(1.0)
        , depth(0.5)
        , waveform(Sine)
        , sampleRate(44100.0)
        , phaseOffset(0.0)
        , currentRate(1.0)
        , rateSmoothing(0.997)
        , beatPosition(0.0)
        , lastBeatPosition(0.0)
    {
        // Initialize smoothed values with optimized timings
        smoothedDepth.reset(sampleRate, 0.02);  // 20ms smoothing for depth
        smoothedRate.reset(sampleRate, 0.05);   // 50ms smoothing for rate
        phaseSmoothing.reset(sampleRate, 0.01); // 10ms smoothing for phase
    }

    void setRate(float newRate)
    {
        rate = newRate;
        smoothedRate.setTargetValue(newRate);
    }

    void setDepth(float newDepth)
    {
        depth = newDepth;
        smoothedDepth.setTargetValue(newDepth);
    }

    void setWaveform(Waveform newWaveform) { waveform = newWaveform; }

    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        rateSmoothing = pow(0.5, 1.0 / (sampleRate * 0.005));
        
        // Update smoothing times and calculate reset transition increment
        smoothedDepth.reset(sampleRate, 0.05);
        smoothedRate.reset(sampleRate, 0.08);
        phaseSmoothing.reset(sampleRate, 0.03);
        resetTransitionIncrement = 1.0f / (resetTransitionTime * static_cast<float>(sampleRate));
    }
    
    void setSyncMode(bool shouldSync, double division = 1.0)
    {
        syncedToHost = shouldSync;
        noteDivision = division;
    }
    
    void setPhaseOffset(float offsetDegrees)
    {
        phaseOffset = offsetDegrees / 360.0f;
    }

    void setBeatPosition(double newBeatPosition)
    {
        lastBeatPosition = beatPosition;
        beatPosition = newBeatPosition;
    }

    void resetPhase()
    {
        phase = 0.0;
        currentRate = rate;
        smoothedDepth.reset(sampleRate, 0.05);
        smoothedRate.reset(sampleRate, 0.05);
    }
    
    bool isWaitingForReset() const { return waitingForReset; }
    
    void updateActiveState(bool isActive, bool isPlaying)
    {
        if (!isPlaying)
        {
            // Immediate reset when playhead stops
            waitingForReset = false;
            wasActive = false;
            phase = 0.0;
            return;
        }

        if (wasActive && !isActive)
        {
            // We just lost audio signal, start waiting for reset
            waitingForReset = true;
        }
        else if (isActive)
        {
            // If we become active again before reset, cancel the wait
            waitingForReset = false;
        }

        wasActive = isActive;
    }
    
    float getNextSample(float waveshapeAmount);

private:
    float calculateCurrentValue(double outputPhase, double smoothedPhase, float waveshapeAmount);
    
    double phase;
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
    
    // Beat sync related
    bool syncedToHost = false;
    double beatPosition;
    double lastBeatPosition;
    double noteDivision = 1.0;
    
    bool waitingForReset = false;  // Flag to track if we're completing a cycle
    bool wasActive = false;        // Track previous active state
    
    float resetTransitionPhase = 0.0f;
    bool inResetTransition = false;
    float lastOutputValue = 0.0f;
    const float resetTransitionTime = 0.05f; // 50ms transition
    float resetTransitionIncrement = 0.0f;
};
