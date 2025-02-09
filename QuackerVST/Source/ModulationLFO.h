/*
  ==============================================================================

    ModulationLFO.h
    Created: 6 Feb 2025 10:17:33pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class ModulationLFO
{
public:
    enum class Waveform {
        Sine,
        Triangle,
        Square,
        Saw
    };

    enum class Target {
        Rate,
        Depth,
        Phase
    };

    struct ValueState {
        float currentValue = 0.0f;
        float previousValue = 0.0f;
        double fraction = 0.0;
    };

    enum class InterpolationType {
        Linear,
        Cubic,
        Hermite
    };

    
    ModulationLFO()
        : phase(0.0)
        , rate(0.5f)
        , depth(0.5f)
        , waveform(Waveform::Sine)
        , sampleRate(44100.0)
        , target(Target::Rate)
    {
        smoothedDepth.reset(sampleRate, 0.02);
        smoothedRate.reset(sampleRate, 0.05);
    }

    void prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        smoothedDepth.reset(sampleRate, 0.02);
        smoothedRate.reset(sampleRate, 0.05);
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
    void setTarget(Target newTarget) { target = newTarget; }
    Target getTarget() const { return target; }
    
    float getNextInterpolatedValue(InterpolationType type = InterpolationType::Linear) {
        float nextValue = getNextValue();
        
        valueState.previousValue = valueState.currentValue;
        valueState.currentValue = nextValue;
        
        valueState.fraction += rateToFraction();
        if (valueState.fraction >= 1.0) {
            valueState.fraction -= 1.0;
        }
        
        switch(type) {
            case InterpolationType::Cubic:
                updateValueHistory(nextValue);
                return cubicInterpolation();
            case InterpolationType::Hermite:
                updateValueHistory(nextValue);
                return hermiteInterpolation();
            default:
                return linearInterpolation();
        }
    }

    // Returns a modulation value between 0 and 1
    float getNextValue()
    {
        float currentRate = smoothedRate.getNextValue();
        phase += currentRate / sampleRate;
        while (phase >= 1.0) phase -= 1.0;

        float rawValue = 0.0f;
        switch (waveform)
        {
            case Waveform::Sine:
                rawValue = 0.5f + 0.5f * std::sin(phase * juce::MathConstants<float>::twoPi);
                break;
            case Waveform::Triangle:
                rawValue = 1.0f - std::abs(2.0f * phase - 1.0f);
                break;
            case Waveform::Square:
                rawValue = phase < 0.5f ? 1.0f : 0.0f;
                break;
            case Waveform::Saw:
                rawValue = phase;
                break;
        }

        return rawValue * smoothedDepth.getNextValue();
    }

    // Apply modulation to a parameter based on the target
    float applyModulation(float baseValue, float modulationValue) const
    {
        switch (target)
        {
            case Target::Rate:
                // Modulate rate between 0.5x and 2x
                return baseValue * (0.5f + 1.5f * modulationValue);
            
            case Target::Depth:
                // Direct depth modulation
                return baseValue * modulationValue;
            
            case Target::Phase:
                // Phase modulation up to ±180 degrees
                return baseValue + (modulationValue - 0.5f) * juce::MathConstants<float>::pi;
            
            default:
                return baseValue;
        }
    }

private:
    double phase;
    float rate;
    float depth;
    Waveform waveform;
    Target target;
    double sampleRate;

    juce::SmoothedValue<float> smoothedDepth;
    juce::SmoothedValue<float> smoothedRate;
    
    ValueState valueState;
    std::array<float, 4> valueHistory = {0.0f, 0.0f, 0.0f, 0.0f};
    static constexpr double INTERPOLATION_FACTOR = 0.5;

    double rateToFraction() {
        return (rate / sampleRate) * INTERPOLATION_FACTOR;
    }

    float linearInterpolation() {
        return valueState.previousValue +
               (valueState.currentValue - valueState.previousValue) *
               valueState.fraction;
    }

    void updateValueHistory(float newValue) {
        // Shift values down
        for (int i = 0; i < 3; ++i) {
            valueHistory[i] = valueHistory[i + 1];
        }
        valueHistory[3] = newValue;
    }

    float cubicInterpolation() {
        float mu = valueState.fraction;
        float mu2 = mu * mu;
        float a0 = valueHistory[3] - valueHistory[2] - valueHistory[0] + valueHistory[1];
        float a1 = valueHistory[0] - valueHistory[1] - a0;
        float a2 = valueHistory[2] - valueHistory[0];
        float a3 = valueHistory[1];

        return (a0 * mu * mu2) + (a1 * mu2) + (a2 * mu) + a3;
    }

    float hermiteInterpolation() {
        float mu = valueState.fraction;
        float mu2 = mu * mu;
        float mu3 = mu2 * mu;
        
        float m0 = (valueHistory[2] - valueHistory[0]) * 0.5f;
        float m1 = (valueHistory[3] - valueHistory[1]) * 0.5f;
        
        float a0 = 2.0f * mu3 - 3.0f * mu2 + 1.0f;
        float a1 = mu3 - 2.0f * mu2 + mu;
        float a2 = mu3 - mu2;
        float a3 = -2.0f * mu3 + 3.0f * mu2;
        
        return a0 * valueHistory[1] + a1 * m0 + a2 * m1 + a3 * valueHistory[2];
    }
    
};
