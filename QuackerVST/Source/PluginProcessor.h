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
    
    
    juce::AudioParameterFloat* lfoRateParam;
    juce::AudioParameterFloat* lfoDepthParam;
    
    juce::AudioParameterChoice* lfoWaveformParam;
    
    juce::AudioParameterBool* lfoSyncParam; // Toggle for syncing
    juce::AudioParameterChoice* lfoNoteDivisionParam; // Note divisions
    
    juce::AudioParameterFloat* lfoPhaseOffsetParam; //Phase offset in degrees (Feel - Rushing/Dragging)
    
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
            // Increment the main phase
            phase += (rate / sampleRate);
            if (phase >= 1.0) phase -= 1.0;

            // Apply phase offset
            double shiftedPhase = phase + phaseOffset;
            
            // Wrap the phase to keep it between 0 and 1
            while (shiftedPhase >= 1.0) shiftedPhase -= 1.0;
            while (shiftedPhase < 0.0) shiftedPhase += 1.0;
            
            // Calculate the output based on the shifted phase
            switch (waveform)
            {
                case Sine:
                    return (std::sin(shiftedPhase * 2.0 * juce::MathConstants<double>::pi) * 0.5f + 0.5f) * depth;
                    
                case Square:
                    return (shiftedPhase < 0.5f ? 1.0f : 0.0f) * depth;
                    
                case Triangle:
                    return (std::abs(2.0f * shiftedPhase - 1.0f)) * depth;
                    
                default:
                    return 0.0f;
            }
        }

    private:
        double phase;
        float rate, depth;
        Waveform waveform;
        double sampleRate;
        double phaseOffset; // Now stored as normalized phase (0 to 1)
    };

    TremoloLFO lfo;                         //creating the lFO using the defined class above

    float currentBPM = 0.0;                 //Defining variable which will hold DAW bpm

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
