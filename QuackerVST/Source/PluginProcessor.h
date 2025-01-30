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
    
private:
    class TremoloLFO
    {
    public:
        enum Waveform { Sine, Square, Triangle };
        TremoloLFO() : phase(0.0), rate(1.0), depth(0.5), waveform(Sine), sampleRate(44100.0) {}

        void setRate(float newRate) { rate = newRate; }
        void setDepth(float newDepth) { depth = newDepth; }
        void setWaveform(Waveform newWaveform) { waveform = newWaveform; }
        void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }

        float getNextSample()
        {
            if (rate == 0.0f) return 1.0f;

            phase += (rate / sampleRate);
            if (phase >= 1.0) phase -= 1.0;

            switch (waveform)
            {
                case Sine:
                    return (std::sin(phase * 2.0 * juce::MathConstants<double>::pi) * 0.5f + 0.5f) * depth;
                case Square:
                    return (phase < 0.5f ? 1.0f : 0.0f) * depth;
                case Triangle:
                    return (std::abs(2.0f * phase - 1.0f)) * depth;
                default:
                    return 0.0f;
            }
        }
    private:
        double phase;
        float rate, depth;
        Waveform waveform;
        double sampleRate;
    };

    TremoloLFO lfo;
    float currentBPM = 0.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
