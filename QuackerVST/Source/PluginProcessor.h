/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModulationLFO.h"

//==============================================================================
/**
*/
class QuackerVSTAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    QuackerVSTAudioProcessor();
    ~QuackerVSTAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    double getCurrentBPM() const;
    

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    bool isPlaying() const { return currentlyPlaying; }
    bool hasAudioInput() const { return audioInputDetected; }
    bool isLfoWaitingForReset() const { return lfo.isWaitingForReset(); }
    
    float getModulationValue() const { return lastModValue; }
    
    float getWaveshapeValue() const { return lastWaveshapeValue; }
    
private:
    
    // DC Filter components
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                  juce::dsp::IIR::Coefficients<float>> dcFilter;
    juce::dsp::ProcessSpec currentSpecs;
    
    class TremoloLFO
    {
    public:
        enum Waveform {
            Sine,
            Square,
            Triangle,
            SawtoothUp,
            SawtoothDown,
            SoftSquare,
            FenderStyle,    // New
            WurlitzerStyle  // New
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
        
        TremoloLFO()
            : phase(0.0)
            , rate(1.0)
            , depth(0.5)
            , waveform(Sine)
            , sampleRate(44100.0)
            , phaseOffset(0.0)
            , currentRate(1.0)
            , rateSmoothing(0.997)
            , beatPosition(0.0)
            , lastBeatPosition(0.0)
        {
            // Initialize smoothed values with optimized timings
            smoothedDepth.reset(sampleRate, 0.02);  // 20ms smoothing for depth
            smoothedRate.reset(sampleRate, 0.05);   // 50ms smoothing for rate
            phaseSmoothing.reset(sampleRate, 0.01); // 10ms smoothing for phase
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
        void setSampleRate(double newSampleRate)
        {
            sampleRate = newSampleRate;
            rateSmoothing = pow(0.5, 1.0 / (sampleRate * 0.005));
            
            // Update smoothing times and calculate reset transition increment
            smoothedDepth.reset(sampleRate, 0.05);
            smoothedRate.reset(sampleRate, 0.08);
            phaseSmoothing.reset(sampleRate, 0.03);
            resetTransitionIncrement = 1.0f / (resetTransitionTime * static_cast<float>(sampleRate));
            valueState = ValueState(); 
        }
        
        void setSyncMode(bool shouldSync, double division = 1.0)
        {
            syncedToHost = shouldSync;
            noteDivision = division;
        }
        
        void setPhaseOffset(float offsetDegrees)
        {
            phaseOffset = offsetDegrees / 360.0f;
        }

        void setBeatPosition(double newBeatPosition)
        {
            lastBeatPosition = beatPosition;
            beatPosition = newBeatPosition;
        }

        void resetPhase()
        {
            phase = 0.0;
            currentRate = rate;
            smoothedDepth.reset(sampleRate, 0.05);
            smoothedRate.reset(sampleRate, 0.05);
        }
        
        bool isWaitingForReset() const { return waitingForReset; }
        
        void updateActiveState(bool isActive, bool isPlaying)
        {
            if (!isPlaying)
            {
                // Immediate reset when playhead stops
                waitingForReset = false;
                wasActive = false;
                phase = 0.0;
                return;
            }

            if (wasActive && !isActive)
            {
                // We just lost audio signal, start waiting for reset
                waitingForReset = true;
            }
            else if (isActive)
            {
                // If we become active again before reset, cancel the wait
                waitingForReset = false;
            }

            wasActive = isActive;
        }
        
        
        float getNextSample(float waveshapeAmount)
        {
            if (!wasActive && !waitingForReset) {
                lastOutputValue = depth;
                return depth;
            }

            float rawOutput = calculateNextSampleValue(waveshapeAmount);
            updateValueHistory(rawOutput);
            
            valueState.previousValue = valueState.currentValue;
            valueState.currentValue = rawOutput;
            
            valueState.fraction += rateToFraction();
            if (valueState.fraction >= 1.0) {
                valueState.fraction -= 1.0;
            }

            float interpolatedOutput;
            switch(currentInterpolation) {
                case InterpolationType::Cubic:
                    interpolatedOutput = cubicInterpolation();
                    break;
                case InterpolationType::Hermite:
                    interpolatedOutput = hermiteInterpolation();
                    break;
                default:
                    interpolatedOutput = linearInterpolation(valueState.previousValue,
                                                           valueState.currentValue,
                                                           valueState.fraction);
            }

            // Apply depth
            float smoothedDepthValue = smoothedDepth.getNextValue();
            return juce::jlimit(0.0f, 1.0f, interpolatedOutput * smoothedDepthValue +
                                           (1.0f - smoothedDepthValue));
        }
        
        void setInterpolationType(InterpolationType type) {
            currentInterpolation = type;
        }
        
    private:
        
        float calculateNextSampleValue(float waveshapeAmount) {
            float output;
            
            if (inResetTransition) {
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

            return output;
        }
        
        // Add helper method before other private members
        float calculateCurrentValue(double outputPhase, double smoothedPhase, float waveshapeAmount) {
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
        
        ValueState valueState;
        std::array<float, 4> valueHistory = {0.0f, 0.0f, 0.0f, 0.0f};
        static constexpr double INTERPOLATION_FACTOR = 0.5;
        InterpolationType currentInterpolation = InterpolationType::Cubic;

        double rateToFraction() {
            return (currentRate / sampleRate) * INTERPOLATION_FACTOR;
        }

        void updateValueHistory(float newValue) {
            for (int i = 0; i < 3; ++i) {
                valueHistory[i] = valueHistory[i + 1];
            }
            valueHistory[3] = newValue;
        }

        float linearInterpolation(float prev, float curr, float frac) {
            return prev + (curr - prev) * frac;
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
        
        double phase;
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
        
        // Beat sync related
        bool syncedToHost = false;
        double beatPosition;
        double lastBeatPosition;
        double noteDivision = 1.0;
        
        bool waitingForReset = false;  // New flag to track if we're completing a cycle
        bool wasActive = false;        // Track previous active state
        
        // Add new member variables at the top of the private section
        float resetTransitionPhase = 0.0f;
        bool inResetTransition = false;
        float lastOutputValue = 0.0f;
        const float resetTransitionTime = 0.05f; // 50ms transition
        float resetTransitionIncrement = 0.0f;
        
        
    };

    TremoloLFO lfo;                         //creating the lFO using the defined class above
    ModulationLFO modLFO;
    ModulationLFO waveshapeLFO;  // Using same ModulationLFO class
    float lastWaveshapeValue = 0.0f;

    float currentBPM = 0.0;                 //Defining variable which will hold DAW bpm
    
    bool currentlyPlaying = false;
    bool audioInputDetected = false;

    juce::HeapBlock<float> lfoValuesBuffer;
 
    float lastModValue = 0.0f;


    

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
