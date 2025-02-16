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
        // BPM represents beats per minute
        // noteDivision represents the fraction of a beat (e.g., 0.25 = quarter note)
        
        // First calculate cycles per minute
        double cyclesPerMinute = (bpm * noteDivision);
        
        // Convert to Hz (cycles per second)
        double frequencyHz = cyclesPerMinute / 60.0;
        
        // Apply a scaling factor to keep higher divisions within a musical range
        // This helps prevent excessive rates at high BPMs with small divisions
        if (noteDivision > 2.0) { // For divisions faster than 1/8 note
            frequencyHz *= 0.75; // Reduce the frequency to keep it more musical
        }
        
        return frequencyHz;
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
