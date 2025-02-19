/*
  ==============================================================================

    TremoloLFO.cpp
    Created: 14 Feb 2025 12:00:03pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/


#include "TremoloLFO.h"

TremoloLFO::TremoloLFO()
    : phase(0.0)
    , accumulatedPhase(0.0)
    , rate(1.0)
    , depth(0.5)
    , waveform(Sine)
    , sampleRate(44100.0)
    , phaseOffset(0.0)
    , currentRate(1.0)
    , rateSmoothing(0.997)
    , beatPosition(0.0)
    , lastBeatPosition(0.0)
    , syncedToHost(false)
    , noteDivision(1.0)
    , currentBPM(120.0)
    , waitingForReset(false)
    , wasActive(false)
    , inResetTransition(false)
    , resetTransitionPhase(0.0f)
    , lastOutputValue(0.0f)
    , resetTransitionTime(0.05f)
    , resetTransitionIncrement(0.0f)
    , oversamplingFactor(4)
{
    smoothedDepth.reset(sampleRate, 0.02);
    smoothedRate.reset(sampleRate, 0.05);
    phaseSmoothing.reset(sampleRate, 0.01);
    oversampledBuffer.resize(oversamplingFactor);
}



void TremoloLFO::setBPM(double bpm) {
    currentBPM = bpm;
    if (syncedToHost) {
        // When in sync mode, update our effective rate based on the new BPM
        double syncedFreq = bpmToFrequency(bpm, noteDivision);
        setRate(static_cast<float>(syncedFreq));
    }
}

void TremoloLFO::setSampleRate(double newSampleRate) {
    sampleRate = newSampleRate;
    rateSmoothing = std::pow(0.5, 1.0 / (sampleRate * 0.005));
    
    smoothedDepth.reset(sampleRate, 0.05);
    smoothedRate.reset(sampleRate, 0.08);
    phaseSmoothing.reset(sampleRate, 0.03);
    resetTransitionIncrement = 1.0f / (resetTransitionTime * static_cast<float>(sampleRate));

    updateOversamplingFactor();
    waveshaper.setSampleRate(newSampleRate);
}

void TremoloLFO::setRate(float newRate) {
    rate = newRate;
    smoothedRate.setTargetValue(newRate);
    updateOversamplingFactor();
}

void TremoloLFO::setDepth(float newDepth) {
    depth = newDepth;
    smoothedDepth.setTargetValue(newDepth);
}

void TremoloLFO::setWaveform(Waveform newWaveform) {
    waveform = newWaveform;
}

float TremoloLFO::getNextSample() {
    if (!wasActive && !waitingForReset) {
        lastOutputValue = depth;
        return depth;
    }

    float output;

    if (waitingForReset) {
        if (!inResetTransition && (getPhaseNormalized() >= 0.99 || getPhaseNormalized() < 0.01)) {
            inResetTransition = true;
            resetTransitionPhase = 0.0f;
            lastOutputValue = calculateCurrentValue(getPhaseWithOffset(), phaseSmoothing.getNextValue());
        }
    }

    if (inResetTransition) {
        output = handleResetTransition();
    } else {
        output = generateOversampledOutput();
    }

    float smoothedDepthValue = smoothedDepth.getNextValue();
    return output * smoothedDepthValue + (1.0f - smoothedDepthValue);
}

// Ensure setSyncMode updates our rate when sync state changes:
void TremoloLFO::setSyncMode(bool shouldSync, double division) {
    // If we're enabling sync, store the current manual rate
    if (!syncedToHost && shouldSync) {
        lastManualRate = rate;
    }
    
    syncedToHost = shouldSync;
    noteDivision = division;
    
    if (shouldSync) {
        // When enabling sync, immediately update rate to match tempo
        double syncedFreq = bpmToFrequency(currentBPM, division);
        setRate(static_cast<float>(syncedFreq));
    } else {
        // When disabling sync, restore the previous manual rate
        setRate(lastManualRate);
    }
}

void TremoloLFO::setPhaseOffset(float offsetDegrees) {
    phaseOffset = offsetDegrees / 360.0f;
}

void TremoloLFO::setBeatPosition(double newBeatPosition) {
    lastBeatPosition = beatPosition;
    beatPosition = newBeatPosition;
}


void TremoloLFO::updateActiveState(bool isActive, bool isPlaying) {
    if (!isPlaying) {
        waitingForReset = false;
        wasActive = false;
        phase = 0.0;
        return;
    }

    if (wasActive && !isActive) {
        waitingForReset = true;
    } else if (isActive) {
        waitingForReset = false;
    }

    wasActive = isActive;
}

void TremoloLFO::resetPhase() {
    phase = 0.0;
    accumulatedPhase = 0.0;
    currentRate = rate;
    smoothedDepth.reset(sampleRate, 0.05);
    smoothedRate.reset(sampleRate, 0.05);
    waveshaper.reset();
}

bool TremoloLFO::isWaitingForReset() const {
    return waitingForReset;
}

double TremoloLFO::getPhaseNormalized() const {
    return std::fmod(accumulatedPhase, 1.0);
}

double TremoloLFO::getPhaseWithOffset() const {
    double outputPhase = getPhaseNormalized() + phaseOffset;
    while (outputPhase >= 1.0) outputPhase -= 1.0;
    while (outputPhase < 0.0) outputPhase += 1.0;
    return outputPhase;
}

float TremoloLFO::generateOversampledOutput() {
    // Get smoothed parameter values
    currentRate = smoothedRate.getNextValue();
    double smoothedPhase = phaseSmoothing.getNextValue();
    
    // We'll use the same phase accumulation logic for both sync and non-sync modes
    // Calculate phase increment based on our current effective rate
    double phaseIncrement = (currentRate / sampleRate) / oversamplingFactor;
    
    // Generate oversampled points using consistent phase accumulation
    for (int i = 0; i < oversamplingFactor; ++i) {
        // Accumulate phase and wrap between 0 and 1
        accumulatedPhase = std::fmod(accumulatedPhase + phaseIncrement, 1.0);
        
        // Calculate the LFO value for this sample
        oversampledBuffer[i] = calculateCurrentValue(getPhaseWithOffset(), smoothedPhase);
    }
    
    // Apply our downsampling filter (moving average)
    float sum = 0.0f;
    for (int i = 0; i < oversamplingFactor; ++i) {
        sum += oversampledBuffer[i];
    }
    
    lastOutputValue = sum / oversamplingFactor;
    return lastOutputValue;
}

float TremoloLFO::handleResetTransition() {
    resetTransitionPhase += resetTransitionIncrement;
    
    if (resetTransitionPhase >= 1.0f) {
        inResetTransition = false;
        waitingForReset = false;
        phase = 0.0;
        accumulatedPhase = 0.0;
        lastOutputValue = depth;
        return depth;
    }
    
    float cosPhase = (1.0f - std::cos(resetTransitionPhase * juce::MathConstants<float>::pi)) * 0.5f;
    return lastOutputValue * (1.0f - cosPhase) + depth * cosPhase;
}

void TremoloLFO::updateOversamplingFactor() {
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

float TremoloLFO::applyWaveshaping(float input) {
    float shapingAmount = waveshaper.getNextShapingValue();
    
    // Convert input from 0-1 range to -1 to 1
    float centered = input * 2.0f - 1.0f;
    
    // Apply waveshaping using a combination of the original and shaped signals
    float shaped = centered + shapingAmount * std::sin(centered * juce::MathConstants<float>::pi);
    
    // Convert back to 0-1 range and limit
    return juce::jlimit(0.0f, 1.0f, (shaped * 0.5f + 0.5f));
}

float TremoloLFO::calculateCurrentValue(double outputPhase, double smoothedPhase) {
    double output = 0.0;
    switch (waveform) {
        case Sine:
            output = std::sin(outputPhase * 2.0 * juce::MathConstants<double>::pi) * 0.5 + 0.5;
            output = output * (1.0 - smoothedPhase) + smoothedPhase *
                std::sin((outputPhase + 0.01) * 2.0 * juce::MathConstants<double>::pi) * 0.5 + 0.5;
            break;
            
        case Square:
            output = outputPhase < 0.5 ? 1.0 : 0.0;
            break;
            
        case Triangle:
            output = 1.0 - std::abs(2.0 * outputPhase - 1.0);
            break;
            
        case SawtoothUp:
            output = 1.0 - outputPhase;
            break;
            
        case SawtoothDown:
            output = 1.0 - outputPhase;
            break;
            
        case SoftSquare:
        {
            const double sharpness = 10.0;
            double centered = outputPhase * 2.0 - 1.0;
            output = 1.0 / (1.0 + std::exp(-sharpness * centered));
        }
            break;
            
        case FenderStyle:
        {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double raw = std::sin(angle) +
                0.1 * std::sin(2.0 * angle) +
                0.05 * std::sin(3.0 * angle);
            output = (raw * 0.4) + 0.5;
            output = std::pow(output, 1.08);
            output = juce::jlimit(0.0, 1.0, output);
        }
            break;
            
        case WurlitzerStyle:
        {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double sineComponent = std::sin(angle);
            double triangleComponent = 2.0 * std::abs(2.0 * (outputPhase - 0.5)) - 1.0;
            output = (0.6 * sineComponent + 0.4 * triangleComponent) * 0.5 + 0.5;
            output = std::pow(output, 0.9);
        }
            break;
    }
    output = applyWaveshaping(static_cast<float>(output));
    
    return static_cast<float>(output);
}
