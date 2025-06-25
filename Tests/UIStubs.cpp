/*
  ==============================================================================

    UIStubs.cpp
    Minimal UI component stubs for headless testing
    
  ==============================================================================
*/

#include <JuceHeader.h>

#ifdef QUACKER_TEST_BUILD

// If the plugin has UI components that need to be compiled but not used,
// we can provide minimal stubs here. This prevents linking errors
// while keeping tests headless.

// Example stub for PluginEditor if it's required by PluginProcessor
#if __has_include("../QuackerVST/Source/PluginEditor.h")

// Create a minimal stub that satisfies the compiler but doesn't do anything
class QuackerVSTAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    QuackerVSTAudioProcessorEditor (juce::AudioProcessor& p)
        : AudioProcessorEditor (&p)
    {
        // Minimal size for testing
        setSize(400, 300);
    }

    ~QuackerVSTAudioProcessorEditor() override {}

    void paint (juce::Graphics& g) override
    {
        // Minimal paint for testing
        g.fillAll(juce::Colours::black);
    }

    void resized() override
    {
        // No-op for testing
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuackerVSTAudioProcessorEditor)
};

#endif // __has_include

#endif // QUACKER_TEST_BUILD