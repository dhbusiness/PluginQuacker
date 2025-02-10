/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModulationLFO.h"
#include "TremoloLFO.h"

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
