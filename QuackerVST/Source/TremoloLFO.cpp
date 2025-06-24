/*
  ==============================================================================

    TremoloLFO.cpp
    Created: 14 Feb 2025 12:00:03pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "TremoloLFO.h"

TremoloLFO::TremoloLFO()
{
    // Initialize with safe defaults
    try {
        smoothedDepth.reset(sampleRate, 0.02);
        smoothedRate.reset(sampleRate, 0.05);
        phaseSmoothing.reset(sampleRate, 0.015);
        oversampledBuffer.resize(oversamplingFactor);
    }
    catch (const std::exception& e) {
        DBG("TremoloLFO initialization error: " + juce::String(e.what()));
        lastError = ErrorCode::BufferAllocationFailed;
    }
}

TremoloLFO::ErrorCode TremoloLFO::setBPM(double bpm) {
    if (!validateBPM(bpm)) {
        lastError = ErrorCode::InvalidBPM;
        currentBPM = 120.0; // Safe default
        DBG("TremoloLFO: Invalid BPM " + juce::String(bpm) + ", using default 120.0");
        return lastError;
    }
    
    currentBPM = bpm;
    
    if (syncedToHost) {
        try {
            double syncedFreq = bpmToFrequency(bpm, noteDivision);
            return setRate(static_cast<float>(syncedFreq));
        }
        catch (...) {
            DBG("Error in TremoloLFO setBPM calculation");
            lastError = ErrorCode::InvalidBPM;
            return setRate(1.0f); // Safe default
        }
    }
    
    lastError = ErrorCode::None;
    return ErrorCode::None;
}

TremoloLFO::ErrorCode TremoloLFO::setSampleRate(double newSampleRate) {
    if (!validateSampleRate(newSampleRate)) {
        lastError = ErrorCode::InvalidSampleRate;
        DBG("TremoloLFO: Invalid sample rate " + juce::String(newSampleRate));
        return lastError;
    }
    
    sampleRate = newSampleRate;
    rateSmoothing = std::pow(0.5, 1.0 / (sampleRate * 0.005));
    
    try {
        smoothedDepth.reset(sampleRate, 0.05);
        smoothedRate.reset(sampleRate, 0.08);
        phaseSmoothing.reset(sampleRate, 0.03);
        resetTransitionIncrement = 1.0f / (resetTransitionTime * static_cast<float>(sampleRate));
        
        auto result = updateOversamplingFactor();
        if (result != ErrorCode::None) {
            return result;
        }
        
        waveshaper.setSampleRate(newSampleRate);
    }
    catch (const std::exception& e) {
        DBG("TremoloLFO setSampleRate error: " + juce::String(e.what()));
        lastError = ErrorCode::InvalidSampleRate;
        return lastError;
    }
    
    lastError = ErrorCode::None;
    return ErrorCode::None;
}

TremoloLFO::ErrorCode TremoloLFO::setRate(float newRate) {
    if (!validateRate(newRate)) {
        lastError = ErrorCode::InvalidRate;
        rate = juce::jlimit(MIN_RATE, MAX_RATE, newRate);
        DBG("TremoloLFO: Rate clamped to " + juce::String(rate));
    } else {
        rate = newRate;
        lastError = ErrorCode::None;
    }
    
    smoothedRate.setTargetValue(rate);
    return updateOversamplingFactor();
}

TremoloLFO::ErrorCode TremoloLFO::setDepth(float newDepth) {
    if (!validateDepth(newDepth)) {
        lastError = ErrorCode::InvalidDepth;
        depth = juce::jlimit(MIN_DEPTH, MAX_DEPTH, newDepth);
        DBG("TremoloLFO: Depth clamped to " + juce::String(depth));
    } else {
        depth = newDepth;
        lastError = ErrorCode::None;
    }
    
    smoothedDepth.setTargetValue(depth);
    return lastError;
}

TremoloLFO::ErrorCode TremoloLFO::setWaveform(Waveform newWaveform) {
    if (!validateWaveform(static_cast<int>(newWaveform))) {
        lastError = ErrorCode::InvalidWaveform;
        DBG("TremoloLFO: Invalid waveform " + juce::String(static_cast<int>(newWaveform)));
        return lastError;
    }
    
    // Safe waveform change with phase reset for specific waveforms
    bool needsPhaseReset = (waveform != newWaveform) &&
        (newWaveform == PulseDecay || newWaveform == GuitarPick ||
         newWaveform == SlowGear || waveform == PulseDecay ||
         waveform == GuitarPick || waveform == SlowGear);
    
    waveform = newWaveform;
    
    if (needsPhaseReset) {
        phase = 0.0;
        accumulatedPhase = 0.0;
    }
    
    lastError = ErrorCode::None;
    return ErrorCode::None;
}

float TremoloLFO::getNextSample() noexcept {
    // This is the real-time audio callback - no exceptions, minimal branching
    if (!wasActive && !waitingForReset) {
        lastOutputValue = depth;
        return depth;
    }

    float output;

    if (waitingForReset) {
        double phaseNorm = getPhaseNormalized();
        if (!inResetTransition && (phaseNorm >= 0.99 || phaseNorm < 0.01)) {
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
    return juce::jlimit(0.0f, 1.0f, output * smoothedDepthValue + (1.0f - smoothedDepthValue));
}

TremoloLFO::ErrorCode TremoloLFO::setSyncMode(bool shouldSync, double division) {
    if (!validateDivision(division)) {
        lastError = ErrorCode::InvalidDivision;
        division = juce::jlimit(MIN_DIVISION, MAX_DIVISION, division);
        DBG("TremoloLFO: Division clamped to " + juce::String(division));
    }
    
    // Store current manual rate before enabling sync
    if (!syncedToHost && shouldSync && rate > 0.0f) {
        lastManualRate = rate;
    }
    
    syncedToHost = shouldSync;
    noteDivision = division;
    
    if (shouldSync) {
        try {
            double syncedFreq = bpmToFrequency(currentBPM > 0.0 ? currentBPM : 120.0, division);
            return setRate(static_cast<float>(syncedFreq));
        }
        catch (...) {
            DBG("Error in TremoloLFO setSyncMode");
            lastError = ErrorCode::InvalidRate;
            return setRate(1.0f);
        }
    } else {
        return setRate(lastManualRate > 0.0f ? lastManualRate : 1.0f);
    }
}

TremoloLFO::ErrorCode TremoloLFO::setPhaseOffset(float offsetDegrees) {
    // Clamp to valid range
    offsetDegrees = juce::jlimit(-360.0f, 360.0f, offsetDegrees);
    phaseOffset = offsetDegrees / 360.0f;
    lastError = ErrorCode::None;
    return ErrorCode::None;
}

void TremoloLFO::setBeatPosition(double newBeatPosition) {
    lastBeatPosition = beatPosition;
    beatPosition = juce::jlimit(0.0, 1e6, newBeatPosition); // Reasonable upper limit
}

void TremoloLFO::updateActiveState(bool isActive, bool isPlaying) noexcept {
    // Safe state transitions
    if (!isPlaying) {
        if (wasActive && !isActive) {
            resetPhase();
        }
        waitingForReset = false;
    } else {
        if (wasActive && !isActive) {
            waitingForReset = true;
        } else if (isActive) {
            waitingForReset = false;
        }
    }
    wasActive = isActive;
}

void TremoloLFO::resetPhase() {
    phase = 0.0;
    accumulatedPhase = 0.0;
    currentRate = (rate > 0.0f) ? rate : 1.0f;
    
    try {
        smoothedDepth.reset(sampleRate, 0.05);
        smoothedRate.reset(sampleRate, 0.05);
        waveshaper.reset();
    }
    catch (...) {
        // Fail silently in real-time context
    }
}

double TremoloLFO::getPhaseNormalized() const noexcept {
    double norm = std::fmod(accumulatedPhase, 1.0);
    return (norm < 0.0) ? norm + 1.0 : norm;
}

double TremoloLFO::getPhaseWithOffset() const noexcept {
    double outputPhase = getPhaseNormalized() + phaseOffset;
    
    // Efficient phase wrapping
    if (outputPhase >= 1.0) {
        outputPhase -= std::floor(outputPhase);
    } else if (outputPhase < 0.0) {
        outputPhase += std::ceil(-outputPhase);
    }
    
    return outputPhase;
}

float TremoloLFO::generateOversampledOutput() noexcept {
    currentRate = smoothedRate.getNextValue();
    double smoothedPhase = phaseSmoothing.getNextValue();
    
    // Safe phase increment calculation
    double phaseIncrement = (currentRate / sampleRate) / oversamplingFactor;
    phaseIncrement = juce::jlimit(0.0, 0.5, phaseIncrement); // Prevent aliasing
    
    // Generate oversampled points
    for (int i = 0; i < oversamplingFactor; ++i) {
        accumulatedPhase = std::fmod(accumulatedPhase + phaseIncrement, 1.0);
        
        // Bounds check for buffer access
        if (i < static_cast<int>(oversampledBuffer.size())) {
            oversampledBuffer[i] = calculateCurrentValue(getPhaseWithOffset(), smoothedPhase);
        }
    }
    
    // Apply downsampling filter (moving average)
    float sum = 0.0f;
    int validSamples = std::min(oversamplingFactor, static_cast<int>(oversampledBuffer.size()));
    
    for (int i = 0; i < validSamples; ++i) {
        sum += oversampledBuffer[i];
    }
    
    lastOutputValue = (validSamples > 0) ? (sum / validSamples) : 0.0f;
    return lastOutputValue;
}

float TremoloLFO::handleResetTransition() noexcept {
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

TremoloLFO::ErrorCode TremoloLFO::updateOversamplingFactor() {
    int newFactor;
    
    if (rate > sampleRate * 0.1) {
        newFactor = 16;
    } else if (rate > sampleRate * 0.05) {
        newFactor = 8;
    } else if (rate > sampleRate * 0.01) {
        newFactor = 4;
    } else {
        newFactor = 2;
    }
    
    if (oversamplingFactor != newFactor) {
        oversamplingFactor = newFactor;
        
        try {
            oversampledBuffer.resize(oversamplingFactor);
        }
        catch (const std::exception& e) {
            DBG("TremoloLFO: Failed to resize oversampled buffer: " + juce::String(e.what()));
            lastError = ErrorCode::BufferAllocationFailed;
            
            // Fall back to minimum oversampling
            oversamplingFactor = 2;
            try {
                oversampledBuffer.resize(oversamplingFactor);
            }
            catch (...) {
                // Critical failure - keep existing buffer
                return lastError;
            }
        }
    }
    
    return ErrorCode::None;
}

float TremoloLFO::applyWaveshaping(float input) noexcept {
    // Clamp input to valid range
    input = juce::jlimit(0.0f, 1.0f, input);
    
    // Map to [-1, 1] for shaping
    float baseValue = input * 2.0f - 1.0f;
    
    // Get shaping value safely
    float shapingValue = waveshaper.getNextShapingValue();
    
    // Combine with limiting
    float combined = juce::jlimit(-1.0f, 1.0f, baseValue + shapingValue);
    
    // Map back to [0, 1]
    return combined * 0.5f + 0.5f;
}

float TremoloLFO::calculateCurrentValue(double outputPhase, double smoothedPhase) noexcept {
    // Ensure phase is in valid range
    outputPhase = juce::jlimit(0.0, 1.0, outputPhase);
    smoothedPhase = juce::jlimit(0.0, 1.0, smoothedPhase);
    
    double output = 0.0;
    
    switch (waveform) {
        case Sine: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            output = std::sin(angle) * 0.5 + 0.5;
            
            // Smooth interpolation
            double nextAngle = (outputPhase + 0.01) * 2.0 * juce::MathConstants<double>::pi;
            double nextValue = std::sin(nextAngle) * 0.5 + 0.5;
            output = output * (1.0 - smoothedPhase) + smoothedPhase * nextValue;
            break;
        }
            
        case Square:
            output = (outputPhase < 0.5) ? 1.0 : 0.0;
            break;
            
        case Triangle:
            output = 1.0 - std::abs(2.0 * outputPhase - 1.0);
            break;
            
        case SawtoothUp:
        case SawtoothDown:
            output = 1.0 - outputPhase;
            break;
            
        case SoftSquare: {
            const double sharpness = 10.0;
            double centered = juce::jlimit(-1.0, 1.0, outputPhase * 2.0 - 1.0);
            output = 1.0 / (1.0 + std::exp(-sharpness * centered));
            break;
        }
            
        case FenderStyle: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double raw = std::sin(angle) +
                        0.1 * std::sin(2.0 * angle) +
                        0.05 * std::sin(3.0 * angle);
            output = (raw * 0.4) + 0.5;
            output = std::pow(juce::jlimit(0.0, 1.0, output), 1.08);
            break;
        }
            
        case WurlitzerStyle: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double sineComponent = std::sin(angle);
            double triangleComponent = 2.0 * std::abs(2.0 * (outputPhase - 0.5)) - 1.0;
            output = (0.6 * sineComponent + 0.4 * triangleComponent) * 0.5 + 0.5;
            output = std::pow(juce::jlimit(0.0, 1.0, output), 0.9);
            break;
        }
            
        case VoxStyle: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double bias = 0.3;
            output = std::sin(angle + bias * std::sin(2.0 * angle)) * 0.5 + 0.5;
            output += 0.1 * std::sin(3.0 * angle);
            break;
        }
            
        case MagnatoneStyle: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double sine = std::sin(angle);
            double parabolic = 1.0 - std::pow(juce::jlimit(-1.0, 1.0, 2.0 * outputPhase - 1.0), 2.0);
            output = (0.7 * sine + 0.3 * parabolic) * 0.5 + 0.5;
            break;
        }
            
        case PulseDecay: {
            const double decayRate = 4.0;
            output = std::exp(-decayRate * outputPhase);
            if (outputPhase < 0.1) {
                output = 1.0 - (outputPhase * 10.0);
            }
            break;
        }
            
        case BouncingBall: {
            double t = juce::jlimit(0.0, 1.0, outputPhase);
            double bounce = std::abs(std::sin(std::pow(t * juce::MathConstants<double>::pi, 0.8)));
            output = std::pow(bounce, 2.0);
            break;
        }
            
        case MultiSine: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            output = std::sin(angle) * 0.5;
            output += std::sin(2.0 * angle) * 0.25;
            output += std::sin(3.0 * angle) * 0.125;
            output = output * 0.5 + 0.5;
            break;
        }
            
        case OpticalStyle: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double response = std::sin(angle);
            if (response < 0) {
                response = response * 0.8;
            }
            response += 0.15 * std::sin(2.0 * angle);
            output = response * 0.5 + 0.5;
            output = std::pow(juce::jlimit(0.0, 1.0, output), 1.2);
            break;
        }
            
        case TwinPeaks: {
            double phase1 = juce::jlimit(0.0, 2.0, outputPhase * 2.0);
            double phase2 = phase1 - 0.5;
            if (phase2 < 0) phase2 += 2.0;
            
            double peak1 = std::exp(-std::pow(phase1 - 0.5, 2) * 16.0);
            double peak2 = std::exp(-std::pow(phase2 - 0.5, 2) * 16.0);
            
            output = (peak1 + peak2 * 0.8) * 0.7;
            break;
        }
            
        case SmoothRandom: {
            // Check cache first
            if (std::abs(outputPhase - waveformCache.lastPhase) < 0.0001) {
                output = waveformCache.cachedValue;
                break;
            }
            
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double f1 = std::sin(angle);
            double f2 = std::sin(angle * 1.47) * 0.5;
            double f3 = std::sin(angle * 2.39) * 0.25;
            double f4 = std::sin(angle * 3.17) * 0.125;
            
            output = (f1 + f2 + f3 + f4) * 0.4 + 0.5;
            
            // Update cache
            waveformCache.lastPhase = outputPhase;
            waveformCache.cachedValue = output;
            break;
        }
            
        case GuitarPick: {
            const double attackTime = 0.05;
            const double decayTime = 0.3;
            
            if (outputPhase < attackTime) {
                output = outputPhase / attackTime;
            } else {
                double decayPhase = juce::jlimit(0.0, 1.0, (outputPhase - attackTime) / decayTime);
                double decay = std::exp(-decayPhase * 3.0);
                double sustain = 0.2;
                output = sustain + (1.0 - sustain) * decay;
            }
            break;
        }
            
        case VintageChorus: {
            double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
            double primary = std::sin(angle);
            double secondary = std::sin(angle * 0.5) * 0.3;
            double harmonics = std::sin(angle * 3.0) * 0.1;
            
            output = (primary + secondary + harmonics) * 0.5 + 0.5;
            break;
        }
            
        case SlowGear: {
            double swell = 1.0 - std::exp(-outputPhase * 4.0);
            double decay = std::exp(-juce::jmax(0.0, outputPhase - 0.7) * 8.0);
            
            if (outputPhase < 0.7) {
                output = swell;
            } else {
                output = swell * decay;
            }
            break;
        }
            
        default:
            output = 0.5; // Safe default
            break;
    }
    
    // Apply waveshaping and ensure valid output range
    output = juce::jlimit(0.0, 1.0, output);
    return applyWaveshaping(static_cast<float>(output));
}

void TremoloLFO::storeManualRate(float manualRate) noexcept {
    lastManualRate = juce::jlimit(MIN_RATE, MAX_RATE, manualRate);
}

TremoloLFO::ErrorCode TremoloLFO::setWaveshapeParameters(float rate, float depth, int waveform, bool enabled) {
    // Validate all parameters
    if (!validateRate(rate)) {
        rate = juce::jlimit(MIN_RATE, MAX_RATE, rate);
    }
    
    if (!validateDepth(depth)) {
        depth = juce::jlimit(MIN_DEPTH, MAX_DEPTH, depth);
    }
    
    if (!validateWaveform(waveform)) {
        waveform = 0; // Default to sine
    }
    
    waveshaper.setRate(rate);
    waveshaper.setDepth(depth);
    waveshaper.setWaveform(static_cast<WaveshapeLFO::Waveform>(waveform));
    waveshaper.setEnabled(enabled);
    
    return ErrorCode::None;
}

// Validation methods
bool TremoloLFO::validateSampleRate(double sr) const noexcept {
    return sr >= MIN_SAMPLE_RATE && sr <= MAX_SAMPLE_RATE;
}

bool TremoloLFO::validateRate(float r) const noexcept {
    return r >= MIN_RATE && r <= MAX_RATE && std::isfinite(r);
}

bool TremoloLFO::validateDepth(float d) const noexcept {
    return d >= MIN_DEPTH && d <= MAX_DEPTH && std::isfinite(d);
}

bool TremoloLFO::validateWaveform(int w) const noexcept {
    return w >= 0 && w < static_cast<int>(NumWaveforms);
}

bool TremoloLFO::validateBPM(double bpm) const noexcept {
    return bpm >= MIN_BPM && bpm <= MAX_BPM && std::isfinite(bpm);
}

bool TremoloLFO::validateDivision(double div) const noexcept {
    return div >= MIN_DIVISION && div <= MAX_DIVISION && std::isfinite(div);
}
