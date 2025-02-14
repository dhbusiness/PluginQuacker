/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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
            , oversamplingFactor(4)  // Default 4x oversampling
        {
            // Initialize smoothed values with optimized timings
            smoothedDepth.reset(sampleRate, 0.02);  // 20ms smoothing for depth
            smoothedRate.reset(sampleRate, 0.05);   // 50ms smoothing for rate
            phaseSmoothing.reset(sampleRate, 0.01); // 10ms smoothing for phase
            
            // Initialize oversampling buffers
            oversampledBuffer.resize(oversamplingFactor);
        }
        
        void setSampleRate(double newSampleRate) {
            sampleRate = newSampleRate;
            rateSmoothing = std::pow(0.5, 1.0 / (sampleRate * 0.005));
            
            // Update smoothing times
            smoothedDepth.reset(sampleRate, 0.05);
            smoothedRate.reset(sampleRate, 0.08);
            phaseSmoothing.reset(sampleRate, 0.03);
            resetTransitionIncrement = 1.0f / (resetTransitionTime * static_cast<float>(sampleRate));

            // Update oversampling if rate is high
            updateOversamplingFactor();
        }
        
        void setRate(float newRate) {
            rate = newRate;
            smoothedRate.setTargetValue(newRate);
            updateOversamplingFactor();  // Adjust oversampling based on new rate
        }

        void setDepth(float newDepth)
        {
            depth = newDepth;
            smoothedDepth.setTargetValue(newDepth);
        }
        
        float getNextSample() {
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

            // Apply depth with smoothing
            float smoothedDepthValue = smoothedDepth.getNextValue();
            return output * smoothedDepthValue + (1.0f - smoothedDepthValue);
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

        void setBeatPosition(double newBeatPosition) {
            if (syncedToHost) {
                lastBeatPosition = beatPosition;
                beatPosition = newBeatPosition;
                
                // Update accumulated phase to match beat position
                double beatsPerCycle = 4.0 / noteDivision;
                accumulatedPhase = std::fmod((beatPosition / beatsPerCycle) * 2.0, 1.0);
            }
        }

        void setWaveform(Waveform newWaveform) { waveform = newWaveform; }
        
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
        
        

        
        void resetPhase() {
            phase = 0.0;
            accumulatedPhase = 0.0;  // Reset accumulated phase as well
            currentRate = rate;
            smoothedDepth.reset(sampleRate, 0.05);
            smoothedRate.reset(sampleRate, 0.05);
        }
        
    private:
        
        // Improved phase handling
        double getPhaseNormalized() const {
            return std::fmod(accumulatedPhase, 1.0);
        }
        
        double getPhaseWithOffset() const {
            double outputPhase = getPhaseNormalized() + phaseOffset;
            while (outputPhase >= 1.0) outputPhase -= 1.0;
            while (outputPhase < 0.0) outputPhase += 1.0;
            return outputPhase;
        }
        
        // Oversampling implementation
        float generateOversampledOutput() {
            float sum = 0.0f;
            
            if (!syncedToHost) {
                // Non-synced mode remains the same - this was working well
                currentRate = smoothedRate.getNextValue();
                double phaseIncrement = (currentRate / sampleRate) / oversamplingFactor;
                
                for (int i = 0; i < oversamplingFactor; ++i) {
                    accumulatedPhase += phaseIncrement;
                    
                    if (accumulatedPhase > 1000.0) {
                        accumulatedPhase = std::fmod(accumulatedPhase, 1.0);
                    }
                    
                    double smoothedPhase = phaseSmoothing.getNextValue();
                    sum += calculateCurrentValue(getPhaseWithOffset(), smoothedPhase);
                }
            } else {
                // Tempo-synced mode - revert to original logic but with oversampling
                double beatsPerCycle = 4.0 / noteDivision;
                double basePhaseDelta = ((beatPosition - lastBeatPosition) / beatsPerCycle) * 2.0;
                double phaseIncrement = basePhaseDelta / oversamplingFactor;
                
                // For tempo sync, we calculate each oversampled position directly
                for (int i = 0; i < oversamplingFactor; ++i) {
                    double currentPhasePosition = (beatPosition + (i * phaseIncrement / 2.0)) / beatsPerCycle * 2.0;
                    accumulatedPhase = std::fmod(currentPhasePosition, 1.0);
                    
                    double smoothedPhase = phaseSmoothing.getNextValue();
                    sum += calculateCurrentValue(getPhaseWithOffset(), smoothedPhase);
                }
            }
            
            lastOutputValue = sum / oversamplingFactor;
            return lastOutputValue;
        }
        
        float handleResetTransition() {
            resetTransitionPhase += resetTransitionIncrement;
            
            if (resetTransitionPhase >= 1.0f) {
                inResetTransition = false;
                waitingForReset = false;
                phase = 0.0;
                accumulatedPhase = 0.0;  // Reset accumulated phase
                lastOutputValue = depth;
                return depth;
            }
            
            // Cosine interpolation for smoother transition
            float cosPhase = (1.0f - std::cos(resetTransitionPhase * juce::MathConstants<float>::pi)) * 0.5f;
            return lastOutputValue * (1.0f - cosPhase) + depth * cosPhase;
        }

        void updateOversamplingFactor() {
            // Dynamically adjust oversampling based on rate
            // Higher rates need more oversampling to prevent aliasing
            if (rate > sampleRate * 0.1) {  // Above 10% of sample rate
                oversamplingFactor = 16;
            } else if (rate > sampleRate * 0.05) {  // Above 5% of sample rate
                oversamplingFactor = 8;
            } else if (rate > sampleRate * 0.01) {  // Above 1% of sample rate
                oversamplingFactor = 4;
            } else {
                oversamplingFactor = 2;
            }
            
            // Resize buffer if needed
            if (oversampledBuffer.size() != oversamplingFactor) {
                oversampledBuffer.resize(oversamplingFactor);
            }
        }
        
        
        
        // Add helper method before other private members
        float calculateCurrentValue(double outputPhase, double smoothedPhase) {
            // Generate waveform
            double output = 0.0;
            switch (waveform)
            {
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
            return output;
        }
        
        // Member variables
        double phase;
        double accumulatedPhase;  // New: accumulated phase for higher precision
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
        
        bool syncedToHost = false;
        double beatPosition;
        double lastBeatPosition;
        double noteDivision = 1.0;
        
        bool waitingForReset = false;
        bool wasActive = false;
        bool inResetTransition = false;
        
        float resetTransitionPhase = 0.0f;
        float lastOutputValue = 0.0f;
        const float resetTransitionTime = 0.05f;
        float resetTransitionIncrement = 0.0f;

        // New: Oversampling-related members
        int oversamplingFactor;
        std::vector<float> oversampledBuffer;
        
        
    };

    TremoloLFO lfo;                         //creating the lFO using the defined class above

    float currentBPM = 0.0;                 //Defining variable which will hold DAW bpm
    
    bool currentlyPlaying = false;
    bool audioInputDetected = false;
    juce::HeapBlock<float> lfoValuesBuffer; 
 


    

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
