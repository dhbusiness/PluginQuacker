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
    
    class TremoloLFO //Defining LFO class
    {
    public:
        
        enum Waveform {Sine, Square, Triangle};
        
        TremoloLFO() : phase(0.0), rate(1.0), depth(0.5), waveform(Sine),sampleRate(44100.0), phaseOffset(0.0) {} //Defining LFO params - phaseOffset added for feel control

        void setRate(float newRate) { rate = newRate; }                             //Creating methods to allows us to set params in the pluginProcessor.cpp
        void setDepth(float newDepth) { depth = newDepth; }                         //Creating methods to allows us to set params in the pluginProcessor.cpp
        void setWaveform(Waveform newWaveform) { waveform = newWaveform; }
        void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }    //Creating methods to allows us to set params in the pluginProcessor.cpp
        void setPhaseOffset(float offsetDegrees) {phaseOffset = offsetDegrees * juce::MathConstants<double>::pi / 180.0;}

        float getNextSample()                                                       //LFO function in here
        {
            phase += (rate / sampleRate);
            if (phase >= 1.0) phase -= 1.0;                                         //Inverts phase if it becomes 1 or greater
            
            double adjustedPhase = phase + phaseOffset / (2.0 * juce::MathConstants<double>::pi); //Calculation for phase offsetting
            if (adjustedPhase >= 1.0) adjustedPhase -= 1.0;
            
            switch (waveform) //Added adjusted phase offset to wave calcs
            {
                case Sine:
                    return (std::sin(adjustedPhase * 2.0 * juce::MathConstants<double>::pi) * 0.5f + 0.5f) * depth;
                    
                case Square:
                    return (adjustedPhase < 0.5f ? 1.0f : 0.0f) * depth;
                    
                case Triangle:
                    return (std::abs(2.0f * adjustedPhase - 1.0f)) * depth;
                    
                default:
                    return 0.0f; // Fallback
            }
        }

    private:
        double phase;                   //Variables for the LFO class
        float rate, depth;
        Waveform waveform;
        double sampleRate;
        double phaseOffset; //Phase offset in radians (Feel control)
    };

    TremoloLFO lfo;                         //creating the lFO using the defined class above

    float currentBPM = 0.0;                 //Defining variable which will hold DAW bpm

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
