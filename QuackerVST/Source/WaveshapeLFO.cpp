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
            
        case VoxStyle:
        {
            // Vox-style bias tremolo emulation with slight asymmetry
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            double bias = 0.3; // Adds characteristic asymmetry
            output = std::sin(angle + bias * std::sin(2.0 * angle)) * 0.5 + 0.5;
            // Add subtle harmonics characteristic of bias tremolo
            output += 0.1 * std::sin(3.0 * angle);
            output = juce::jlimit(0.0, 1.0, output);
        }
        break;

        case MagnatoneStyle:
        {
            // Magnatone-style pitch-vibrato inspired waveshape
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            // Combine sine and parabolic shaping for that distinctive "pitch-varying" quality
            double sine = std::sin(angle);
            double parabolic = 1.0 - std::pow(2.0 * phase - 1.0, 2.0);
            output = (0.7 * sine + 0.3 * parabolic) * 0.5 + 0.5;
        }
        break;
            
        case PulseDecay:
        {
            // Creates a sharp attack with exponential decay
            double decayRate = 4.0; // Adjust for different decay characteristics
            phase = std::exp(-decayRate * phase);
            if (phase < 0.1) // Sharp attack phase
                output = 1.0 - (phase * 10.0);
            output = juce::jlimit(0.0, 1.0, output);
        }
        break;

        case BouncingBall:
        {
            // Simulates the timing of a bouncing ball
            double t = phase;
            double bounce = std::abs(std::sin(std::pow(t * juce::MathConstants<double>::pi, 0.8)));
            output = std::pow(bounce, 2.0); // Adds natural-feeling acceleration
        }
        break;
            
        case MultiSine:
        {
            // Combines multiple sine waves for rich modulation
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            output = std::sin(angle) * 0.5;
            output += std::sin(2.0 * angle) * 0.25;
            output += std::sin(3.0 * angle) * 0.125;
            output = output * 0.5 + 0.5;
        }
        break;
            
        case OpticalStyle:
        {
            // Emulates the response of optical tremolo circuits like those found in vintage Fender amps
            // Characterized by a smooth, asymmetrical response due to the photocell behavior
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            double response = std::sin(angle);
            // Add photocell-like "lag" on the decay
            if (response < 0) {
                response = response * 0.8;  // Slower decay
            }
            // Add subtle harmonics characteristic of optical circuits
            response += 0.15 * std::sin(2.0 * angle);
            output = response * 0.5 + 0.5;
            output = std::pow(output, 1.2); // Slight non-linear shaping
        }
        break;

        case TwinPeaks:
        {
            // Creates a distinctive dual-peak wave that produces a "double-pulse" effect
            // Useful for rhythmic tremolo effects
            double phase1 = phase * 2.0;
            double phase2 = phase1 - 0.5;
            if (phase2 < 0) phase2 += 2.0;
            
            // Create two gaussian-like peaks
            double peak1 = std::exp(-std::pow(phase1 - 0.5, 2) * 16.0);
            double peak2 = std::exp(-std::pow(phase2 - 0.5, 2) * 16.0);
            
            output = (peak1 + peak2 * 0.8) * 0.7; // Second peak slightly lower
        }
        break;

        case SmoothRandom:
        {
            // First check cache before doing any calculations
            if (std::abs(phase - waveformCache.lastPhase) < 0.0001) {
                output = waveformCache.cachedValue;
                break;  // Exit early if using cached value
            }
            
            // Calculate the base waveform
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            double f1 = std::sin(angle);
            double f2 = std::sin(angle * 1.47) * 0.5;  // Non-integer relationship creates pseudo-random feel
            double f3 = std::sin(angle * 2.39) * 0.25;
            double f4 = std::sin(angle * 3.17) * 0.125;
            
            // Calculate output and normalize to 0-1 range
            output = (f1 + f2 + f3 + f4) * 0.4 + 0.5;
            output = juce::jlimit(0.0, 1.0, output);
            
            // Cache the final, limited value
            waveformCache.lastPhase = phase;
            waveformCache.cachedValue = output;
        }
        break;

        case GuitarPick:
        {
            // Emulates the envelope of a picked guitar string
            // Fast attack, natural decay, slight sustain
            const double attackTime = 0.05;
            const double decayTime = 0.3;
            
            if (phase < attackTime) {
                // Sharp attack
                output = phase / attackTime;
            } else {
                // Natural decay with slight sustain
                double decayPhase = (phase - attackTime) / decayTime;
                double decay = std::exp(-decayPhase * 3.0);
                double sustain = 0.2;  // Sustain level
                output = sustain + (1.0 - sustain) * decay;
            }
        }
        break;

        case VintageChorus:
        {
            // Inspired by the LFO shapes found in vintage chorus units
            // Creates a more "dimensional" modulation
            double angle = phase * 2.0 * juce::MathConstants<double>::pi;
            double primary = std::sin(angle);
            double secondary = std::sin(angle * 0.5) * 0.3;  // Slower secondary modulation
            
            // Add subtle higher harmonics for richness
            double harmonics = std::sin(angle * 3.0) * 0.1;
            
            output = (primary + secondary + harmonics) * 0.5 + 0.5;
            output = juce::jlimit(0.0, 1.0, output);
        }
        break;

        case SlowGear:
        {
            // Inspired by the Boss Slow Gear pedal's envelope
            // Creates a gradual swell effect
            double swell = 1.0 - std::exp(-phase * 4.0);
            double decay = std::exp(-(phase - 0.7) * 8.0);
            
            if (phase < 0.7) {
                output = swell;
            } else {
                output = swell * decay;
            }
        }
        break;
            
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
