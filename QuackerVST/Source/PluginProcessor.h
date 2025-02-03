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
        {
            // Initialize smoothed values
            smoothedDepth.reset(44100, 0.05); // 50ms smoothing
            smoothedRate.reset(44100, 0.05);
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
            // Update smoothed values sample rate
            smoothedDepth.reset(sampleRate, 0.05);
            smoothedRate.reset(sampleRate, 0.05);
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
        
        float getNextSample()
        {
            // Smooth the rate for free-running mode
            if (!syncedToHost)
            {
                currentRate = smoothedRate.getNextValue();
                phase += currentRate / sampleRate;
                
                // Wrap phase
                while (phase >= 1.0) phase -= 1.0;
                while (phase < 0.0) phase += 1.0;
            }
            else
            {
                // In sync mode, calculate phase directly from beat position
                double beatsPerCycle = 4.0 / noteDivision; // 4 for whole note, 2 for half, 1 for quarter, etc.
                phase = std::fmod((beatPosition / beatsPerCycle) * 2.0, 1.0); // Multiply by 2 to complete full cycle per interval
            }

            // Apply phase offset
            double outputPhase = phase + phaseOffset;
            while (outputPhase >= 1.0) outputPhase -= 1.0;
            while (outputPhase < 0.0) outputPhase += 1.0;

            // Generate waveform
            double output = 0.0;
            switch (waveform)
            {
                case Sine:
                    output = std::sin(outputPhase * 2.0 * juce::MathConstants<double>::pi) * 0.5 + 0.5;
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
                        // Create a softer square wave using sigmoid function
                        const double sharpness = 10.0; // Adjust this to control the softness
                        double centered = outputPhase * 2.0 - 1.0;
                        output = 1.0 / (1.0 + std::exp(-sharpness * centered));
                    }
                    break;
                    
                case FenderStyle:
                {
                    // Fender-style tremolo: Asymmetric sine wave with subtle harmonics
                    double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
                    
                    // Calculate the base waveform with proper scaling
                    double raw = std::sin(angle) +                     // Fundamental
                                 0.1 * std::sin(2.0 * angle) +         // 2nd harmonic
                                 0.05 * std::sin(3.0 * angle);         // 3rd harmonic
                    
                    // Normalize to 0-1 range with headroom
                    output = (raw * 0.4) + 0.5;  // Scale by 0.4 to ensure we stay within bounds
                    
                    // Add subtle asymmetry without causing dropouts
                    output = std::pow(output, 1.08);
                    
                    // Ensure output stays within bounds
                    output = juce::jlimit(0.0, 1.0, output);
                }
                break;

                case WurlitzerStyle:
                    {
                        // Wurlitzer-style: Blend of triangle and sine with peak emphasis
                        double angle = outputPhase * 2.0 * juce::MathConstants<double>::pi;
                        double sineComponent = std::sin(angle);
                        double triangleComponent = 2.0 * std::abs(2.0 * (outputPhase - 0.5)) - 1.0;
                        
                        // Blend the components (60% sine, 40% triangle)
                        output = (0.6 * sineComponent + 0.4 * triangleComponent) * 0.5 + 0.5;
                        
                        // Add slight emphasis to peaks
                        output = std::pow(output, 0.9);
                    }
                    break;
            
            }

            return output * smoothedDepth.getNextValue() + (1.0f - smoothedDepth.getNextValue());
        }
        
        void setSyncMode(bool shouldSync, double division = 1.0)
        {
            syncedToHost = shouldSync;
            noteDivision = division;
        }

    private:
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
        
        // Beat sync related
        bool syncedToHost = false;
        double beatPosition;
        double lastBeatPosition;
        double noteDivision = 1.0;
    };

    TremoloLFO lfo;                         //creating the lFO using the defined class above

    float currentBPM = 0.0;                 //Defining variable which will hold DAW bpm
    
    bool currentlyPlaying = false;
    bool audioInputDetected = false;

    
    

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
