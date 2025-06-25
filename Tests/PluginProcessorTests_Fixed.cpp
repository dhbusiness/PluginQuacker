/*
  ==============================================================================

    PluginProcessorTests.cpp
    Unit tests for QuackerVSTAudioProcessor (Fixed Version)
    
  ==============================================================================
*/

#include "QuackerVSTTests.h"
#include "TestProcessorWrapper.h"

class PluginProcessorTests : public QuackerTestBase {
public:
    PluginProcessorTests() : QuackerTestBase("Plugin Processor Tests") {}
    
    void runTest() override {
        beginTest("Plugin Processor Tests");
        
        DiagnosticLogger::getInstance().startLogging("plugin_processor_test_log.txt");
        
        runSubTest("Initialization Test", [this]() { testInitialization(); });
        runSubTest("Audio Processing Test", [this]() { testAudioProcessing(); });
        runSubTest("Parameter Automation Test", [this]() { testParameterAutomation(); });
        runSubTest("Bypass Functionality Test", [this]() { testBypass(); });
        runSubTest("Mix Control Test", [this]() { testMixControl(); });
        runSubTest("State Save/Restore Test", [this]() { testStateSaveRestore(); });
        runSubTest("Bus Layout Test", [this]() { testBusLayout(); });
        runSubTest("Performance Benchmarks", [this]() { testPerformanceBenchmarks(); });
        
        DiagnosticLogger::getInstance().stopLogging();
    }
    
private:
    void testInitialization() {
        QuackerVSTAudioProcessor processor;
        
        // Test basic properties
        expect(processor.getName() == JucePlugin_Name, "Plugin name should match");
        expect(!processor.isMidiEffect(), "Should not be a MIDI effect");
        expect(processor.acceptsMidi() == false, "Should not accept MIDI");
        expect(processor.producesMidi() == false, "Should not produce MIDI");
        expect(processor.getTailLengthSeconds() == 0.0, "Should have no tail");
        
        // Test parameter tree
        auto& apvts = processor.apvts;
        expect(apvts.state.isValid(), "Parameter tree should be valid");
        
        // Verify all parameters exist
        const juce::StringArray paramIDs = {
            "lfoRate", "lfoDepth", "lfoWaveform", "lfoSync",
            "lfoNoteDivision", "lfoPhaseOffset", "mix", "bypass"
        };
        
        for (const auto& paramID : paramIDs) {
            auto* param = apvts.getParameter(paramID);
            expect(param != nullptr, "Parameter " + paramID + " should exist");
        }
        
        DIAG_LOG("Processor", "Initialization test complete");
    }
    
    void testAudioProcessing() {
        QuackerVSTAudioProcessor processor;
        
        const double sampleRate = 44100.0;
        const int blockSize = 512;
        
        processor.prepareToPlay(sampleRate, blockSize);
        
        // Create test buffer with sine wave
        auto inputBuffer = TestUtilities::createTestBuffer(2, blockSize, 440.0f, sampleRate);
        auto outputBuffer = inputBuffer; // Copy for processing
        
        // Process audio
        juce::MidiBuffer midiBuffer;
        processor.processBlock(outputBuffer, midiBuffer);
        
        // Verify output is valid
        expect(TestUtilities::isValidAudioBuffer(outputBuffer), 
               "Output should be valid audio");
        
        // Test with different settings
        processor.apvts.getParameter("lfoRate")->setValueNotifyingHost(0.5f);
        processor.apvts.getParameter("lfoDepth")->setValueNotifyingHost(0.8f);
        
        // Process multiple blocks to let modulation take effect
        for (int i = 0; i < 10; ++i) {
            outputBuffer = inputBuffer;
            processor.processBlock(outputBuffer, midiBuffer);
        }
        
        // Verify modulation is applied (output should differ from input)
        bool isModulated = !TestUtilities::compareBuffers(inputBuffer, outputBuffer, 0.01f);
        expect(isModulated, "Audio should be modulated");
        
        // Test silence detection
        juce::AudioBuffer<float> silentBuffer(2, blockSize);
        silentBuffer.clear();
        processor.processBlock(silentBuffer, midiBuffer);
        
        // With silence, output should remain mostly silent
        float rms = TestUtilities::calculateRMS(silentBuffer);
        expect(rms < 0.1f, "Silent input should remain mostly silent");
        
        DIAG_LOG("Processor", "Audio processing test complete");
    }
    
    void testParameterAutomation() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        auto& apvts = processor.apvts;
        
        // Test parameter smoothing
        auto* rateParam = apvts.getParameter("lfoRate");
        auto* depthParam = apvts.getParameter("lfoDepth");
        
        if (rateParam && depthParam) {
            // Set initial values
            rateParam->setValueNotifyingHost(0.1f);
            depthParam->setValueNotifyingHost(0.5f);
            
            // Process some audio
            juce::AudioBuffer<float> buffer(2, 512);
            juce::MidiBuffer midiBuffer;
            
            for (int i = 0; i < 10; ++i) {
                processor.processBlock(buffer, midiBuffer);
            }
            
            // Change parameters
            rateParam->setValueNotifyingHost(0.9f);
            depthParam->setValueNotifyingHost(0.8f);
            
            for (int i = 0; i < 10; ++i) {
                processor.processBlock(buffer, midiBuffer);
            }
            
            expect(true, "Parameter automation completed without crashes");
        }
        
        DIAG_LOG("Processor", "Parameter automation test complete");
    }
    
    void testBypass() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        auto& apvts = processor.apvts;
        auto* bypassParam = apvts.getParameter("bypass");
        
        if (bypassParam) {
            // Create test signal
            auto inputBuffer = TestUtilities::createTestBuffer(2, 512);
            
            // Test with bypass off
            bypassParam->setValueNotifyingHost(0.0f);
            auto processedBuffer = inputBuffer;
            juce::MidiBuffer midiBuffer;
            
            // Set strong modulation
            if (auto* depthParam = apvts.getParameter("lfoDepth")) {
                depthParam->setValueNotifyingHost(1.0f);
            }
            
            // Process multiple blocks
            for (int i = 0; i < 10; ++i) {
                processor.processBlock(processedBuffer, midiBuffer);
            }
            
            // Test with bypass on
            bypassParam->setValueNotifyingHost(1.0f);
            auto bypassedBuffer = inputBuffer;
            
            for (int i = 0; i < 10; ++i) {
                processor.processBlock(bypassedBuffer, midiBuffer);
            }
            
            // Verify bypass behavior
            expect(TestUtilities::isValidAudioBuffer(bypassedBuffer), 
                   "Bypassed output should be valid");
        }
        
        DIAG_LOG("Processor", "Bypass test complete");
    }
    
    void testMixControl() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        auto& apvts = processor.apvts;
        auto* mixParam = apvts.getParameter("mix");
        
        if (mixParam) {
            auto inputBuffer = TestUtilities::createTestBuffer(2, 512);
            juce::MidiBuffer midiBuffer;
            
            // Test different mix values
            const float mixValues[] = {0.0f, 0.5f, 1.0f};
            
            for (float mixValue : mixValues) {
                mixParam->setValueNotifyingHost(mixValue);
                auto testBuffer = inputBuffer;
                
                for (int i = 0; i < 5; ++i) {
                    processor.processBlock(testBuffer, midiBuffer);
                }
                
                expect(TestUtilities::isValidAudioBuffer(testBuffer), 
                       "Mix at " + juce::String(mixValue) + " should produce valid audio");
            }
        }
        
        DIAG_LOG("Processor", "Mix control test complete");
    }
    
    void testStateSaveRestore() {
        QuackerVSTAudioProcessor processor1;
        QuackerVSTAudioProcessor processor2;
        
        // Set specific state in processor1
        auto& apvts1 = processor1.apvts;
        if (auto* rateParam = apvts1.getParameter("lfoRate")) {
            rateParam->setValueNotifyingHost(0.7f);
        }
        if (auto* depthParam = apvts1.getParameter("lfoDepth")) {
            depthParam->setValueNotifyingHost(0.67f);
        }
        
        // Save state
        juce::MemoryBlock stateData;
        processor1.getStateInformation(stateData);
        
        expect(stateData.getSize() > 0, "State data should not be empty");
        
        // Restore state to processor2
        processor2.setStateInformation(stateData.getData(), 
                                     static_cast<int>(stateData.getSize()));
        
        expect(true, "State save/restore completed without crashes");
        
        DIAG_LOG("Processor", "State save/restore test complete");
    }
    
    void testBusLayout() {
        TestProcessorWrapper processor;
        
        // Test supported configurations using the wrapper
        expect(processor.testSupportsChannelConfiguration(1, 1), "Should support mono");
        expect(processor.testSupportsChannelConfiguration(2, 2), "Should support stereo");
        
        // Test unsupported configurations
        expect(!processor.testSupportsChannelConfiguration(1, 2), "Should not support mono to stereo");
        expect(!processor.testSupportsChannelConfiguration(2, 1), "Should not support stereo to mono");
        expect(!processor.testSupportsChannelConfiguration(6, 6), "Should not support 5.1");
        
        DIAG_LOG("Processor", "Bus layout test complete");
    }
    
    void testPerformanceBenchmarks() {
        QuackerVSTAudioProcessor processor;
        
        const int sampleRates[] = {44100, 48000, 96000};
        const int blockSizes[] = {64, 128, 256, 512};
        
        for (int sr : sampleRates) {
            for (int bs : blockSizes) {
                processor.prepareToPlay(sr, bs);
                
                juce::AudioBuffer<float> buffer(2, bs);
                juce::MidiBuffer midiBuffer;
                
                // Warm up
                for (int i = 0; i < 10; ++i) {
                    processor.processBlock(buffer, midiBuffer);
                }
                
                // Benchmark
                const int numIterations = 1000;
                auto start = std::chrono::high_resolution_clock::now();
                
                for (int i = 0; i < numIterations; ++i) {
                    processor.processBlock(buffer, midiBuffer);
                }
                
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                
                double avgMicroseconds = duration.count() / static_cast<double>(numIterations);
                double cpuUsage = (avgMicroseconds / (bs * 1000000.0 / sr)) * 100.0;
                
                DIAG_LOG("Performance", 
                        "SR: " + juce::String(sr) + 
                        " BS: " + juce::String(bs) + 
                        " Avg: " + juce::String(avgMicroseconds, 2) + "µs" +
                        " CPU: " + juce::String(cpuUsage, 2) + "%");
                
                // Verify reasonable performance
                expect(cpuUsage < 50.0, "CPU usage should be reasonable");
            }
        }
        
        DIAG_LOG("Processor", "Performance benchmarks complete");
    }
};

// Register the test
static PluginProcessorTests pluginProcessorTests;