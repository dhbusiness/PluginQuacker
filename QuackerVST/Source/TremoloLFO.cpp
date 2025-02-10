/*
  ==============================================================================

    TremoloLFO.cpp
    Created: 10 Feb 2025 2:46:54pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "TremoloLFO.h"

float TremoloLFO::getNextSample(float waveshapeAmount)
{
    if (!wasActive && !waitingForReset) {
        lastOutputValue = depth;
        return depth;
    }

    float output;
    
    if (waitingForReset) {
        if (!inResetTransition && (phase >= 0.99 || phase < 0.01)) {
            inResetTransition = true;
            resetTransitionPhase = 0.0f;
            double outputPhase = phase + phaseOffset;
            while (outputPhase >= 1.0) outputPhase -= 1.0;
            while (outputPhase < 0.0) outputPhase += 1.0;
            double smoothedPhase = phaseSmoothing.getNextValue();
            lastOutputValue = calculateCurrentValue(outputPhase, smoothedPhase, waveshapeAmount);
        }
    }

    if (inResetTransition) {
        // Smoothly transition from last value to reset position
        resetTransitionPhase += resetTransitionIncrement;
        
        if (resetTransitionPhase >= 1.0f) {
            inResetTransition = false;
            waitingForReset = false;
            phase = 0.0;
            lastOutputValue = depth;
            output = depth;
        } else {
            float cosPhase = (1.0f - std::cos(resetTransitionPhase * juce::MathConstants<float>::pi)) * 0.5f;
            output = lastOutputValue * (1.0f - cosPhase) + depth * cosPhase;
        }
    } else {
        // Normal LFO operation
        if (!syncedToHost) {
            currentRate = smoothedRate.getNextValue();
            phase += currentRate / sampleRate;
            while (phase >= 1.0) phase -= 1.0;
        } else {
            double beatsPerCycle = 4.0 / noteDivision;
            phase = std::fmod((beatPosition / beatsPerCycle) * 2.0, 1.0);
        }
        
        double outputPhase = phase + phaseOffset;
        while (outputPhase >= 1.0) outputPhase -= 1.0;
        while (outputPhase < 0.0) outputPhase += 1.0;
        double smoothedPhase = phaseSmoothing.getNextValue();
        output = calculateCurrentValue(outputPhase, smoothedPhase, waveshapeAmount);
        lastOutputValue = output;
    }

    // Apply depth
    float smoothedDepthValue = smoothedDepth.getNextValue();
    return output * smoothedDepthValue + (1.0f - smoothedDepthValue);
}

float TremoloLFO::calculateCurrentValue(double outputPhase, double smoothedPhase, float waveshapeAmount)
{
    // Generate waveform
    double output = 0.0;
    switch (waveform)
    {
        case Sine:
        {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            output = std::sin(angle);
            
            // Apply waveshaping modulation
            if (waveshapeAmount > 0.0f) {
                // Add harmonics based on waveshape amount
                output += waveshapeAmount * 0.3 * std::sin(2.0 * angle);  // 2nd harmonic
                output += waveshapeAmount * 0.15 * std::sin(3.0 * angle); // 3rd harmonic
                output = output / (1.0 + waveshapeAmount * 0.45); // Normalize
            }
            
            output = output * 0.5 + 0.5; // Convert to unipolar
            
            // Apply phase smoothing
            output = output * (1.0 - smoothedPhase) + smoothedPhase *
                (std::sin((outputPhase + 0.01) * 2.0 * juce::MathConstants<double>::pi) * 0.5 + 0.5);
        }
        break;
            
        case Square:
        {
            // Apply waveshaping to adjust duty cycle and edge softness
            double threshold = 0.5 + (waveshapeAmount * 0.4 * std::sin(outputPhase * 6.28318));
            double softness = 0.01 + waveshapeAmount * 0.2;
            output = 1.0 / (1.0 + std::exp(-(outputPhase - threshold) / softness));
        }
        break;
            
        case Triangle:
        {
            output = 1.0 - std::abs(2.0 * outputPhase - 1.0);
            
            // Apply waveshaping for asymmetric triangle
            if (waveshapeAmount > 0.0f) {
                double shaped = std::pow(output, 1.0 + waveshapeAmount * 2.0);
                output = output * (1.0 - waveshapeAmount) + shaped * waveshapeAmount;
            }
        }
        break;
            
        case SawtoothUp:
        {
            output = outputPhase;
            // Add exponential shaping
            if (waveshapeAmount > 0.0f) {
                double shaped = std::pow(output, 1.0 + waveshapeAmount * 2.0);
                output = output * (1.0 - waveshapeAmount) + shaped * waveshapeAmount;
            }
        }
        break;
            
        case SawtoothDown:
        {
            output = 1.0 - outputPhase;
            // Add exponential shaping
            if (waveshapeAmount > 0.0f) {
                double shaped = std::pow(output, 1.0 + waveshapeAmount * 2.0);
                output = output * (1.0 - waveshapeAmount) + shaped * waveshapeAmount;
            }
        }
        break;
            
        case SoftSquare:
        {
            const double baseSharpness = 10.0;
            double modifiedSharpness = baseSharpness * (1.0 - waveshapeAmount * 0.8);
            double centered = outputPhase * 2.0 - 1.0;
            output = 1.0 / (1.0 + std::exp(-modifiedSharpness * centered));
        }
        break;
            
        case FenderStyle:
        {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            
            // Modulate harmonic content
            double h1 = std::sin(angle);
            double h2 = std::sin(2.0 * angle) * (0.1 + waveshapeAmount * 0.2);
            double h3 = std::sin(3.0 * angle) * (0.05 + waveshapeAmount * 0.15);
            double h4 = waveshapeAmount * 0.1 * std::sin(4.0 * angle);
            
            double raw = h1 + h2 + h3 + h4;
            output = (raw * 0.4) + 0.5;
            output = std::pow(output, 1.08 + waveshapeAmount * 0.4);
            output = juce::jlimit(0.0, 1.0, output);
        }
        break;
            
        case WurlitzerStyle:
        {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double sineComponent = std::sin(angle);
            double triangleComponent = 2.0 * std::abs(2.0 * (outputPhase - 0.5)) - 1.0;
            
            // Modulate the balance between sine and triangle components
            double balance = 0.6 + waveshapeAmount * 0.3;
            output = (balance * sineComponent + (1.0 - balance) * triangleComponent) * 0.5 + 0.5;
            output = std::pow(output, 0.9 + waveshapeAmount * 0.2);
        }
        break;
    }
    
    return output;
}
