/*
  ==============================================================================

    WaveshapeLFO.cpp
    Created: 19 Feb 2025 10:55:51am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "WaveshapeLFO.h"

WaveshapeLFO::WaveshapeLFO()
    : phase(0.0)
    , rate(1.0f)
    , depth(0.5f)
    , waveform(Sine)
    , sampleRate(44100.0)
    , isEnabled(false)
    , oversamplingFactor(4)
    , lastOutputValue(0.0f)
{
    smoothedDepth.reset(sampleRate, 0.02);
    smoothedRate.reset(sampleRate, 0.05);
    oversampledBuffer.resize(oversamplingFactor);
}



void WaveshapeLFO::setSampleRate(double newSampleRate) {
    sampleRate = newSampleRate;
    smoothedDepth.reset(sampleRate, 0.05);
    smoothedRate.reset(sampleRate, 0.08);
    updateOversamplingFactor();
}

void WaveshapeLFO::setRate(float newRate) {
    rate = newRate;
    smoothedRate.setTargetValue(newRate);
    updateOversamplingFactor();
}

void WaveshapeLFO::setDepth(float newDepth) {
    depth = newDepth;
    smoothedDepth.setTargetValue(newDepth);
}

void WaveshapeLFO::setWaveform(Waveform newWaveform) {
    waveform = newWaveform;
}

void WaveshapeLFO::setEnabled(bool shouldBeEnabled) {
    isEnabled = shouldBeEnabled;
    if (!isEnabled) {
        lastOutputValue = 0.0f;
    }
}

float WaveshapeLFO::getNextShapingValue() {
    if (!isEnabled) {
        // Smooth transition when disabling
        lastOutputValue *= 0.99f;
        if (std::abs(lastOutputValue) < 0.0001f) {
            lastOutputValue = 0.0f;
        }
        return lastOutputValue;
    }
    
    return generateOversampledOutput();
}

float WaveshapeLFO::generateOversampledOutput() {
    float currentRate = smoothedRate.getNextValue();
    double phaseIncrement = (currentRate / sampleRate) / oversamplingFactor;
    
    for (int i = 0; i < oversamplingFactor; ++i) {
        phase = std::fmod(phase + phaseIncrement, 1.0);
        oversampledBuffer[i] = calculateCurrentValue(phase);
    }
    
    float sum = 0.0f;
    for (int i = 0; i < oversamplingFactor; ++i) {
        sum += oversampledBuffer[i];
    }
    
    lastOutputValue = (sum / oversamplingFactor) * smoothedDepth.getNextValue();
    return lastOutputValue;
}

float WaveshapeLFO::calculateCurrentValue(double phase) {
    double output = 0.0;
    switch (waveform) {
        case Sine:
            output = std::sin(phase * 2.0 * juce::MathConstants<double>::pi);
            break;
            
        case Square:
            output = phase < 0.5 ? 1.0 : -1.0;
            break;
            
        case Triangle:
            output = 2.0 * (phase < 0.5 ? phase * 2.0 : 2.0 * (1.0 - phase)) - 1.0;
            break;
            
        case SawtoothUp:
            output = 2.0 * phase - 1.0;
            break;
            
        case SawtoothDown:
            output = 1.0 - 2.0 * phase;
            break;
            
        case SoftSquare: {
            const double sharpness = 10.0;
            double centered = phase * 2.0 - 1.0;
            output = 2.0 * (1.0 / (1.0 + std::exp(-sharpness * centered))) - 1.0;
            break;
        }
            
        case FenderStyle: {
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            output = std::sin(angle) +
                    0.1 * std::sin(2.0 * angle) +
                    0.05 * std::sin(3.0 * angle);
            output = juce::jlimit(-1.0, 1.0, output);
            break;
        }
            
        case WurlitzerStyle: {
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            double sineComponent = std::sin(angle);
            double triangleComponent = 2.0 * (phase < 0.5 ? phase * 2.0 : 2.0 * (1.0 - phase)) - 1.0;
            output = 0.6 * sineComponent + 0.4 * triangleComponent;
            break;
        }
    }
    return static_cast<float>(output);
}

void WaveshapeLFO::updateOversamplingFactor() {
    // Adjust oversampling based on rate to maintain quality
    if (rate > sampleRate * 0.1) {
        oversamplingFactor = 16;
    } else if (rate > sampleRate * 0.05) {
        oversamplingFactor = 8;
    } else if (rate > sampleRate * 0.01) {
        oversamplingFactor = 4;
    } else {
        oversamplingFactor = 2;
    }
    
    if (oversampledBuffer.size() != oversamplingFactor) {
        oversampledBuffer.resize(oversamplingFactor);
    }
}

void WaveshapeLFO::reset() {
    phase = 0.0;
    lastOutputValue = 0.0f;
    smoothedDepth.reset(sampleRate, 0.05);
    smoothedRate.reset(sampleRate, 0.05);
}
