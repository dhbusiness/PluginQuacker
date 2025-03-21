/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TremoloLFO.h"
#include "PresetManager.h"

class QuackerVSTAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorParameter::Listener {
public:
    QuackerVSTAudioProcessor();
    ~QuackerVSTAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};
    
    double getCurrentBPM() const;
    double getSafeBPM() const {
        // Triple-layered safety check
        double safeBPM = currentBPM;
        if (safeBPM <= 0.0) {
            safeBPM = lastKnownGoodBPM;
            if (safeBPM <= 0.0) {
                safeBPM = defaultBPM;
            }
        }
        return std::max(1.0, safeBPM); // Always return at least 1.0
    };
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    bool isPlaying() const { return currentlyPlaying; }
    bool hasAudioInput() const { return audioInputDetected; }
    bool isLfoWaitingForReset() const { return lfo.isWaitingForReset(); }
    
    PresetManager& getPresetManager() { return *presetManager; }
    void loadFactoryPresets();
    void applyParametersInOrder();

    void syncParametersAfterPresetLoad();
    

    
private:
    // DC Filter components
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                  juce::dsp::IIR::Coefficients<float>> dcFilter;
    juce::dsp::ProcessSpec currentSpecs;
    
    TremoloLFO lfo;
    float currentBPM = 0.0;
    bool currentlyPlaying = false;
    bool audioInputDetected = false;
    juce::HeapBlock<float> lfoValuesBuffer;
    
    // BPM handling
    double defaultBPM = 120.0;
    double lastKnownGoodBPM = 0.0;
    
    bool wasInSync = false;
    
    std::unique_ptr<PresetManager> presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuackerVSTAudioProcessor)
};
