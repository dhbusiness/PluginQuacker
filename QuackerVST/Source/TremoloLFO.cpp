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
    phaseSmoothing.reset(sampleRate, 0.015);
    oversampledBuffer.resize(oversamplingFactor);
}



void TremoloLFO::setBPM(double bpm) {
    // Safety check
    if (bpm <= 0.0) {
        bpm = 120.0; // Default fallback
        juce::Logger::writeToLog("TremoloLFO received invalid BPM, using default: 120.0");
    }
    
    currentBPM = bpm;
    
    if (syncedToHost) {
        try {
            // Calculate synced frequency based on BPM and division
            double syncedFreq = bpmToFrequency(bpm, noteDivision);
            setRate(static_cast<float>(syncedFreq));
        }
        catch (const std::exception& e) {
            // Handle any calculation errors
            juce::Logger::writeToLog("Error in TremoloLFO setBPM: " + juce::String(e.what()));
            setRate(1.0f); // Safe default
        }
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
    // If changing to or from certain waveforms, we might want to reset the phase
    bool needsPhaseReset = (waveform != newWaveform) &&
        (newWaveform == PulseDecay || newWaveform == GuitarPick ||
         newWaveform == SlowGear || waveform == PulseDecay ||
         waveform == GuitarPick || waveform == SlowGear);
    
    waveform = newWaveform;
    
    if (needsPhaseReset) {
        phase = 0.0;
        accumulatedPhase = 0.0;
    }
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
    // Safety check for division
    if (division <= 0.0) {
        division = 1.0; // Default to quarter note
        juce::Logger::writeToLog("TremoloLFO received invalid division, using default: 1.0");
    }
    
    // Store current manual rate before enabling sync
    if (!syncedToHost && shouldSync) {
        lastManualRate = rate > 0.0f ? rate : 1.0f; // Safety check
    }
    
    syncedToHost = shouldSync;
    noteDivision = division;
    
    if (shouldSync) {
        try {
            // Calculate synced frequency based on BPM and division
            double syncedFreq = bpmToFrequency(currentBPM > 0.0 ? currentBPM : 120.0, division);
            setRate(static_cast<float>(syncedFreq));
        }
        catch (const std::exception& e) {
            // Handle any calculation errors
            juce::Logger::writeToLog("Error in TremoloLFO setSyncMode: " + juce::String(e.what()));
            setRate(1.0f); // Safe default
        }
    } else {
        // Restore previous manual rate with safety check
        setRate(lastManualRate > 0.0f ? lastManualRate : 1.0f);
    }
}

void TremoloLFO::setPhaseOffset(float offsetDegrees) {
    phaseOffset = offsetDegrees / 360.0f;
}

void TremoloLFO::setBeatPosition(double newBeatPosition) {
    lastBeatPosition = beatPosition;
    beatPosition = newBeatPosition;
}


void TremoloLFO::updateActiveState(bool isActive, bool isPlaying)
{
    // If transport is not playing, we should still process audio
    // but we need to be careful about phase resets
    if (!isPlaying)
    {
        // Only reset phase if we're going from active to inactive
        if (wasActive && !isActive)
        {
            resetPhase();
        }
        waitingForReset = false;
    }
    else
    {
        // Original behavior for when transport is playing
        if (wasActive && !isActive)
        {
            waitingForReset = true;
        }
        else if (isActive)
        {
            waitingForReset = false;
        }
    }

    wasActive = isActive;
}

void TremoloLFO::resetPhase() {
    phase = 0.0;
    accumulatedPhase = 0.0;
    
    // Safety check for rate
    currentRate = rate > 0.0f ? rate : 1.0f;
    
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
    // Map the base LFO output from [0, 1] to [-1, 1]
    float baseValue = input * 2.0f - 1.0f;
    
    // Get the shaping value from the waveshaper (which handles oversampling and smoothing)
    float shapingValue = waveshaper.getNextShapingValue();
    
    // Combine the base value with the shaping value using simple addition
    float combined = baseValue + shapingValue;
    
    // Clamp the result to the expected range [-1, 1]
    combined = juce::jlimit(-1.0f, 1.0f, combined);
    
    // Map back to [0, 1] for further processing
    return combined * 0.5f + 0.5f;
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
            
        case VoxStyle:
        {
            // Vox-style bias tremolo emulation with slight asymmetry
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
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
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            // Combine sine and parabolic shaping for that distinctive "pitch-varying" quality
            double sine = std::sin(angle);
            double parabolic = 1.0 - std::pow(2.0 * outputPhase - 1.0, 2.0);
            output = (0.7 * sine + 0.3 * parabolic) * 0.5 + 0.5;
        }
        break;
            
        case PulseDecay:
        {
            // Creates a sharp attack with exponential decay
            double decayRate = 4.0; // Adjust for different decay characteristics
            output = std::exp(-decayRate * outputPhase);
            if (outputPhase < 0.1) // Sharp attack phase
                output = 1.0 - (outputPhase * 10.0);
            output = juce::jlimit(0.0, 1.0, output);
        }
        break;

        case BouncingBall:
        {
            // Simulates the timing of a bouncing ball
            double t = outputPhase;
            double bounce = std::abs(std::sin(std::pow(t * juce::MathConstants<double>::pi, 0.8)));
            output = std::pow(bounce, 2.0); // Adds natural-feeling acceleration
        }
        break;
            
        case MultiSine:
        {
            // Combines multiple sine waves for rich modulation
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
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
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
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
            double phase1 = outputPhase * 2.0;
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
            if (std::abs(outputPhase - waveformCache.lastPhase) < 0.0001) {
                output = waveformCache.cachedValue;
                break;  // Exit early if using cached value
            }
            
            // Calculate the base waveform
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double f1 = std::sin(angle);
            double f2 = std::sin(angle * 1.47) * 0.5;  // Non-integer relationship creates pseudo-random feel
            double f3 = std::sin(angle * 2.39) * 0.25;
            double f4 = std::sin(angle * 3.17) * 0.125;
            
            // Calculate output and normalize to 0-1 range
            output = (f1 + f2 + f3 + f4) * 0.4 + 0.5;
            output = juce::jlimit(0.0, 1.0, output);
            
            // Cache the final, limited value
            waveformCache.lastPhase = outputPhase;
            waveformCache.cachedValue = output;
        }
        break;

        case GuitarPick:
        {
            // Emulates the envelope of a picked guitar string
            // Fast attack, natural decay, slight sustain
            const double attackTime = 0.05;
            const double decayTime = 0.3;
            
            if (outputPhase < attackTime) {
                // Sharp attack
                output = outputPhase / attackTime;
            } else {
                // Natural decay with slight sustain
                double decayPhase = (outputPhase - attackTime) / decayTime;
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
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
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
            double swell = 1.0 - std::exp(-outputPhase * 4.0);
            double decay = std::exp(-(outputPhase - 0.7) * 8.0);
            
            if (outputPhase < 0.7) {
                output = swell;
            } else {
                output = swell * decay;
            }
        }
        break;
            
            
    }
    output = applyWaveshaping(static_cast<float>(output));
    
    return static_cast<float>(output);
}
