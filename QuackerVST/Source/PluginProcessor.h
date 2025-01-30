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
    
private:
    
    class TremoloLFO
    {
    public:
        enum Waveform {Sine, Square, Triangle};
        
        TremoloLFO() : phase(0.0), rate(1.0), depth(0.5), waveform(Sine), sampleRate(44100.0), phaseOffset(0.0) {}

        void setRate(float newRate) { rate = newRate; }
        void setDepth(float newDepth) { depth = newDepth; }
        void setWaveform(Waveform newWaveform) { waveform = newWaveform; }
        void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }
        
        // Updated phase offset to work in degrees (-180 to +180)
        void setPhaseOffset(float offsetDegrees)
        {
            // Convert degrees to normalized phase (0 to 1)
            phaseOffset = offsetDegrees / 360.0f;
        }

        float getNextSample()
        {
            // Calculate phase increment
            double phaseIncrement = rate / sampleRate;
            
            // Update phase
            phase += phaseIncrement;
            
            // Wrap phase between 0 and 1
            while (phase >= 1.0) phase -= 1.0;
            
            // Calculate the offset phase for output
            double outputPhase = phase + phaseOffset;
            while (outputPhase >= 1.0) outputPhase -= 1.0;
            while (outputPhase < 0.0) outputPhase += 1.0;
            
            // Generate output based on current waveform
            switch (waveform)
            {
                case Sine:
                    // Sine goes from -1 to 1, we scale and shift to 0 to 1
                    return (std::sin(outputPhase * 2.0 * juce::MathConstants<double>::pi) * 0.5f + 0.5f) * depth;
                    
                case Square:
                    // Square wave should alternate between full volume and reduced volume
                    return (outputPhase < 0.5f ? 1.0f : 0.0f) * depth + (1.0f - depth);
                    
                case Triangle:
                    // Triangle wave from 0 to 1
                    return (std::abs(2.0f * outputPhase - 1.0f)) * depth + (1.0f - depth);
                    
                default:
                    return 0.0f;
            }
        }

    private:
        double phase = 0.0;
        float rate = 1.0;
        float depth = 0.5;
        Waveform waveform = Sine;
        double sampleRate = 44100.0;
        double phaseOffset = 0.0;
    };

    TremoloLFO lfo;                         //creating the lFO using the defined class above

    float currentBPM = 0.0;                 //Defining variable which will hold DAW bpm

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
