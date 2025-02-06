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
    
    
};
