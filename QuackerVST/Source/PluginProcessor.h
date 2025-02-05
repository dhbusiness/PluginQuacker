/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class QuackerVSTAudioProcessor : public juce::AudioProcessor
{
public:
    QuackerVSTAudioProcessor();
    ~QuackerVSTAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    double getCurrentBPM() const;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    bool isPlaying() const { return currentlyPlaying; }
    bool hasAudioInput() const { return audioInputDetected; }
    bool isLfoWaitingForReset() const { return lfo.isWaitingForReset(); }
    
private:
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
            FenderStyle,
            WurlitzerStyle
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
            , stateFade(0.0f)
        {
            // Initialize smoothed values with longer smoothing times
            smoothedDepth.reset(44100, 0.1);    // 100ms smoothing
            smoothedRate.reset(44100, 0.1);
            smoothedPhaseOffset.reset(44100, 0.1);
            stateFade.reset(44100, 0.01);      // 10ms fade time
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
            // Longer smoothing times for stability
            smoothedDepth.reset(sampleRate, 0.2);    // 200ms smoothing
            smoothedRate.reset(sampleRate, 0.2);
            smoothedPhaseOffset.reset(sampleRate, 0.2);
            stateFade.reset(sampleRate, 0.05);       // 50ms fade

            // Set up oversampling
            oversamplingRatio = (sampleRate < 96000.0) ? 4 : 2;
            
            // Update anti-aliasing filter
            double nyquist = sampleRate * 0.5;
            double filterFreq = std::min(18000.0, nyquist * 0.8);
            updateAntiAliasingFilter(filterFreq);
        }
        
        void setPhaseOffset(float offsetDegrees)
        {
            phaseOffset = offsetDegrees / 360.0f;
            smoothedPhaseOffset.setTargetValue(phaseOffset);
        }

        void setBeatPosition(double newBeatPosition)
        {
            if (std::abs(newBeatPosition - lastBeatPosition) > 1.0) {
                phase = std::fmod(phase, 1.0); // More precise phase reset
            }
            lastBeatPosition = beatPosition;
            beatPosition = newBeatPosition;
        }

        void resetPhase()
        {
            phase = 0.0;
            currentRate = rate;
            smoothedDepth.reset(sampleRate, 0.1);
            smoothedRate.reset(sampleRate, 0.1);
            stateFade.setTargetValue(0.0f);
        }
        
        bool isWaitingForReset() const { return waitingForReset; }
        bool isOversamplingEnabled() const { return oversamplingEnabled; }
        
        void updateActiveState(bool isActive, bool isPlaying)
        {
            if (!isPlaying)
            {
                stateFade.setTargetValue(0.0f);
                waitingForReset = false;
                wasActive = false;
                phase = std::fmod(phase, 1.0);
                return;
            }

            if (wasActive && !isActive)
            {
                waitingForReset = true;
                stateFade.setTargetValue(0.0f);
            }
            else if (isActive)
            {
                waitingForReset = false;
                stateFade.setTargetValue(1.0f);
            }

            wasActive = isActive;
        }
        
        float getNextSample()
        {
            if (waitingForReset || wasActive)
            {
                // Oversample the LFO generation
                float sum = 0.0f;
                for (int i = 0; i < oversamplingRatio; ++i)
                {
                    if (!syncedToHost)
                    {
                        currentRate = smoothedRate.getNextValue();
                        phase = std::fmod(phase + (currentRate / (sampleRate * oversamplingRatio)), 1.0);
                    }
                    else
                    {
                        double beatsPerCycle = 4.0 / noteDivision;
                        phase = std::fmod((beatPosition / beatsPerCycle) * 2.0, 1.0);
                    }

                    double outputPhase = std::fmod(phase + smoothedPhaseOffset.getNextValue(), 1.0);
                    sum += generateWaveform(outputPhase);
                }

                float output = sum / oversamplingRatio;
                float fadeValue = stateFade.getNextValue();
                float depthValue = smoothedDepth.getNextValue();

                // Apply soft saturation to prevent harsh modulation
                output = std::tanh(output * 1.5f) * 0.666f;
                
                return (output * depthValue + (1.0f - depthValue)) * fadeValue;
            }
            return smoothedDepth.getNextValue();
        }
        
        void setSyncMode(bool shouldSync, double division = 1.0)
        {
            syncedToHost = shouldSync;
            noteDivision = division;
        }

    private:
        double generateWaveform(double phase)
        {
            switch (waveform)
            {
                case Sine:
                    return std::sin(phase * 2.0 * juce::MathConstants<double>::pi) * 0.5 + 0.5;
                    
                case Square:
                    return phase < 0.5 ? 1.0 : 0.0;
                    
                case Triangle:
                    return 1.0 - std::abs(2.0 * phase - 1.0);
                    
                case SawtoothUp:
                case SawtoothDown:
                    return 1.0 - phase;
                    
                case SoftSquare:
                {
                    const double sharpness = 10.0;
                    double centered = phase * 2.0 - 1.0;
                    return 1.0 / (1.0 + std::exp(-sharpness * centered));
                }
                    
                case FenderStyle:
                {
                    double angle = phase * 2.0 * juce::MathConstants<double>::pi;
                    double raw = std::sin(angle) + 0.1 * std::sin(2.0 * angle) + 0.05 * std::sin(3.0 * angle);
                    double output = (raw * 0.4) + 0.5;
                    output = std::pow(output, 1.08);
                    return juce::jlimit(0.0, 1.0, output);
                }
                    
                case WurlitzerStyle:
                {
                    double angle = phase * 2.0 * juce::MathConstants<double>::pi;
                    double sineComponent = std::sin(angle);
                    double triangleComponent = 2.0 * std::abs(2.0 * (phase - 0.5)) - 1.0;
                    double output = (0.6 * sineComponent + 0.4 * triangleComponent) * 0.5 + 0.5;
                    return std::pow(output, 0.9);
                }
                    
                default:
                    return 0.0;
            }
        }

        // Anti-aliasing filter
        void updateAntiAliasingFilter(double frequency)
        {
            // Simple one-pole lowpass filter
            double w0 = 2.0 * juce::MathConstants<double>::pi * frequency / sampleRate;
            filterCoeff = std::exp(-w0);
            lastFilterOutput = 0.0;
        }
        
        double filterCoeff;
        double lastFilterOutput;
        int oversamplingRatio;
        
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
        juce::SmoothedValue<float> smoothedPhaseOffset;
        juce::SmoothedValue<float> stateFade;
        
        bool syncedToHost = false;
        double beatPosition;
        double lastBeatPosition;
        double noteDivision = 1.0;
        
        bool waitingForReset = false;
        bool wasActive = false;
        
        // Oversampling
        bool oversamplingEnabled = false;

    };

    TremoloLFO lfo;
    std::atomic<float> currentBPM{0.0f};
    std::atomic<bool> currentlyPlaying{false};
    std::atomic<bool> audioInputDetected{false};
    
    // DSP processors
    juce::dsp::ProcessSpec dspSpec;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> antiAliasingFilter;
    
    static constexpr int RING_BUFFER_SIZE = 4096;
    juce::AbstractFifo fifo{RING_BUFFER_SIZE};
    std::array<float, RING_BUFFER_SIZE> ringBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
