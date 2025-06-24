/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TremoloLFO.h"
#include "PresetManager.h"

class QuackerVSTAudioProcessor : public juce::AudioProcessor,
                                 public juce::AudioProcessorParameter::Listener {
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
    
    double getCurrentBPM() const noexcept { return getSafeBPM(); }
    
    double getSafeBPM() const noexcept {
        double safeBPM = currentBPM.load();
        if (safeBPM <= 0.0) {
            safeBPM = lastKnownGoodBPM.load();
            if (safeBPM <= 0.0) {
                safeBPM = defaultBPM;
            }
        }
        return juce::jmax(1.0, safeBPM);
    }
    
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    bool isPlaying() const noexcept { return currentlyPlaying.load(); }
    bool hasAudioInput() const noexcept { return audioInputDetected.load(); }
    bool isLfoWaitingForReset() const noexcept { return lfo.isWaitingForReset(); }
    
    PresetManager& getPresetManager() noexcept { return *presetManager; }
    void loadFactoryPresets();
    void applyParametersInOrder();
    void syncParametersAfterPresetLoad();
    
    // Error reporting for UI
    struct ProcessorError {
        enum Type {
            None = 0,
            InvalidSampleRate,
            InvalidBPM,
            BufferAllocationFailed,
            DCFilterInitFailed,
            PresetLoadFailed,
            ParameterError
        };
        
        Type type = None;
        juce::String message;
        juce::Time timestamp;
    };
    
    ProcessorError getLastError() const noexcept {
        const juce::ScopedLock sl(errorLock);
        return lastError;
    }
    
    void clearError() noexcept {
        const juce::ScopedLock sl(errorLock);
        lastError = ProcessorError();
    }

private:
    // DC Filter components
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                  juce::dsp::IIR::Coefficients<float>> dcFilter;
    juce::dsp::ProcessSpec currentSpecs;
    
    TremoloLFO lfo;
    
    // Thread-safe atomic values
    std::atomic<double> currentBPM{120.0};
    std::atomic<double> lastKnownGoodBPM{120.0};
    std::atomic<bool> currentlyPlaying{false};
    std::atomic<bool> audioInputDetected{false};
    
    juce::HeapBlock<float> lfoValuesBuffer;
    size_t lfoBufferSize = 0;
    
    // Constants
    static constexpr double defaultBPM = 120.0;
    static constexpr double minValidBPM = 1.0;
    static constexpr double maxValidBPM = 999.0;
    static constexpr float audioDetectionThreshold = 0.0001f;
    static constexpr int maxBlockSize = 8192; // Safety limit
    
    bool wasInSync = false;
    
    std::unique_ptr<PresetManager> presetManager;
    
    // Error handling
    mutable juce::CriticalSection errorLock;
    ProcessorError lastError;
    
    // Helper methods
    void reportError(ProcessorError::Type type, const juce::String& message) noexcept;
    bool validateAudioSpecs(double sampleRate, int samplesPerBlock) const noexcept;
    bool allocateLFOBuffer(int samplesPerBlock) noexcept;
    void processParameterUpdates() noexcept;
    bool validateParameterValue(const juce::String& paramID, float value) const noexcept;
    
    // Safe parameter access
    template<typename T>
    T getSafeParameterValue(const juce::String& paramID, T defaultValue) const noexcept {
        try {
            if (auto* param = apvts.getRawParameterValue(paramID)) {
                return static_cast<T>(param->load());
            }
        }
        catch (...) {
            DBG("Failed to get parameter: " + paramID);
        }
        return defaultValue;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuackerVSTAudioProcessor)
};
