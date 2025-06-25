/*
  ==============================================================================

    SimpleTestRunner.cpp
    Simplified test runner that works without full JUCE CMake integration
    
  ==============================================================================
*/

#include "SimplifiedTests.h"

// Basic tests using the simplified framework
class BasicLFOTest : public SimpleTest {
public:
    BasicLFOTest() : SimpleTest("Basic LFO Test") {}
    
    void runTest() override {
        logMessage("Testing basic LFO functionality...");
        
        #ifdef TREMOLO_LFO_H_INCLUDED
        TremoloLFO lfo;
        
        // Test initialization
        expect(lfo.setSampleRate(44100.0) == TremoloLFO::ErrorCode::None, "Should set valid sample rate");
        expect(lfo.setRate(5.0f) == TremoloLFO::ErrorCode::None, "Should set valid rate");
        expect(lfo.setDepth(0.8f) == TremoloLFO::ErrorCode::None, "Should set valid depth");
        
        // Test sample generation
        float sample1 = lfo.getNextSample();
        float sample2 = lfo.getNextSample();
        
        expect(std::isfinite(sample1), "First sample should be finite");
        expect(std::isfinite(sample2), "Second sample should be finite");
        expect(sample1 >= 0.0f && sample1 <= 1.0f, "Sample should be in range [0,1]");
        
        logMessage("LFO basic functionality test complete");
        #else
        logMessage("TremoloLFO header not available - skipping LFO tests");
        #endif
    }
};

class BasicProcessorTest : public SimpleTest {
public:
    BasicProcessorTest() : SimpleTest("Basic Processor Test") {}
    
    void runTest() override {
        logMessage("Testing basic processor functionality...");
        
        #ifdef PLUGINPROCESSOR_H_INCLUDED
        QuackerVSTAudioProcessor processor;
        
        // Test basic properties
        expect(processor.getName().isNotEmpty(), "Processor should have a name");
        expect(!processor.isMidiEffect(), "Should not be a MIDI effect");
        expect(processor.getNumParameters() > 0, "Should have parameters");
        
        // Test audio processing setup
        processor.prepareToPlay(44100.0, 512);
        
        // Test with simple audio
        juce::AudioBuffer<float> buffer = TestUtils::createSineWave(2, 512);
        juce::MidiBuffer midiBuffer;
        
        processor.processBlock(buffer, midiBuffer);
        
        expect(TestUtils::isValidAudio(buffer), "Processed audio should be valid");
        
        logMessage("Processor basic functionality test complete");
        #else
        logMessage("PluginProcessor header not available - skipping processor tests");
        #endif
    }
};

class BasicParameterTest : public SimpleTest {
public:
    BasicParameterTest() : SimpleTest("Basic Parameter Test") {}
    
    void runTest() override {
        logMessage("Testing basic parameter functionality...");
        
        #ifdef PLUGINPROCESSOR_H_INCLUDED
        QuackerVSTAudioProcessor processor;
        
        auto& apvts = processor.apvts;
        
        // Test that key parameters exist
        auto* rateParam = apvts.getParameter("lfoRate");
        auto* depthParam = apvts.getParameter("lfoDepth");
        auto* mixParam = apvts.getParameter("mix");
        
        expect(rateParam != nullptr, "Rate parameter should exist");
        expect(depthParam != nullptr, "Depth parameter should exist");
        expect(mixParam != nullptr, "Mix parameter should exist");
        
        if (rateParam && depthParam && mixParam) {
            // Test parameter ranges
            rateParam->setValueNotifyingHost(0.5f);
            depthParam->setValueNotifyingHost(0.7f);
            mixParam->setValueNotifyingHost(0.9f);
            
            expect(true, "Parameters accept values without crashing");
        }
        
        logMessage("Parameter test complete");
        #else
        logMessage("PluginProcessor header not available - skipping parameter tests");
        #endif
    }
};

// Register tests
REGISTER_TEST(BasicLFOTest)
REGISTER_TEST(BasicProcessorTest)
REGISTER_TEST(BasicParameterTest)

// Simple main function
int main(int argc, char* argv[]) {
    std::cout << "QuackerVST Simple Test Runner" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Check command line arguments
    bool verbose = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--verbose") {
            verbose = true;
        }
    }
    
    if (verbose) {
        std::cout << "Running in verbose mode..." << std::endl;
    }
    
    // Initialize JUCE
    juce::ScopedJuceInitialiser_GUI juceInit;
    
    // Run all tests
    TestRegistry::getInstance().runAllTests();
    
    return TestRegistry::getInstance().getExitCode();
}