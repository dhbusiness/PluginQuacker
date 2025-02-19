/*
  ==============================================================================

    WaveshapeLFO.h
    Created: 19 Feb 2025 10:51:27am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class WaveshapeLFO {
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

    WaveshapeLFO();
    ~WaveshapeLFO() = default;
    
    // Function declarations only (no implementations)
    void setSampleRate(double newSampleRate);
    void setRate(float newRate);
    void setDepth(float newDepth);
    void setWaveform(Waveform newWaveform);
    void setEnabled(bool shouldBeEnabled);
    float getNextShapingValue();
    void reset();
    
    static float normalizedToRate(float normalizedValue);
    static float rateToNormalized(float rate);

private:
    float calculateCurrentValue(double phase);
    float generateOversampledOutput();
    void updateOversamplingFactor();
    
    double phase = 0.0;
    float rate = 1.0f;
    float depth = 0.5f;
    Waveform waveform = Sine;
    double sampleRate = 44100.0;
    bool isEnabled = false;
    
    juce::SmoothedValue<float> smoothedDepth;
    juce::SmoothedValue<float> smoothedRate;
    
    int oversamplingFactor = 4;
    std::vector<float> oversampledBuffer;
    float lastOutputValue = 0.0f;
    
    juce::HeapBlock<float> waveshapeBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveshapeLFO)
};
