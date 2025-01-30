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
        {}

        void setRate(float newRate)
        {
            rate = newRate;
        }

        void setDepth(float newDepth) { depth = newDepth; }
        void setWaveform(Waveform newWaveform) { waveform = newWaveform; }
        void setSampleRate(double newSampleRate)
        {
            sampleRate = newSampleRate;
            rateSmoothing = pow(0.5, 1.0 / (sampleRate * 0.005));
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

        float getNextSample()
        {
            // Smooth the rate for free-running mode
            if (!syncedToHost)
            {
                currentRate = currentRate * rateSmoothing + rate * (1.0f - rateSmoothing);
                phase += currentRate / sampleRate;
                
                // Wrap phase
                while (phase >= 1.0) phase -= 1.0;
                while (phase < 0.0) phase += 1.0;
            }
            else
            {
                // In sync mode, calculate phase directly from beat position
                double beatsPerCycle = 4.0 / noteDivision; // 4 for whole note, 2 for half, 1 for quarter, etc.
                phase = std::fmod(beatPosition / beatsPerCycle, 1.0);
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
            }

            return output * depth + (1.0f - depth);
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
        
        // Beat sync related
        bool syncedToHost = false;
        double beatPosition;
        double lastBeatPosition;
        double noteDivision = 1.0;
    };

    TremoloLFO lfo;                         //creating the lFO using the defined class above

    float currentBPM = 0.0;                 //Defining variable which will hold DAW bpm

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessor)
};
