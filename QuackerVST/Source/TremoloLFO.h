/*
  ==============================================================================

    TremoloLFO.h
    Created: 14 Feb 2025 12:00:03pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

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
        WurlitzerStyle
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
    static double bpmToFrequency(double bpm, double noteDivision) {
        // noteDivision is in terms of beats (e.g., 0.25 = quarter note, 0.5 = half note)
        return (bpm / 60.0) * noteDivision;
    };

    // Get the current effective frequency, whether synced or not
    double getCurrentEffectiveRate() const {
        if (syncedToHost) {
            return bpmToFrequency(currentBPM, noteDivision);
        }
        return rate;
    };
    
    bool isSynced() const { return syncedToHost; };
    double getCurrentDivision() const { return noteDivision; };
    
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
};
