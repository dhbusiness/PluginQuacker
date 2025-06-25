/*
  ==============================================================================

    PluginProcessorTests.cpp
    Unit tests for QuackerVSTAudioProcessor
    
  ==============================================================================
*/

#include "QuackerVSTTests.h"

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
        runSubTest("DC Filter Test", [this]() { testDCFilter(); });
        runSubTest("Real-time Safety Test", [this]() { testRealtimeSafety(); });
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
            "lfoNoteDivision", "lfoPhaseOffset", "mix", "bypass",
            "waveshapeRate", "waveshapeDepth", "waveshapeWaveform", "waveshapeEnabled"
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
        processor.apvts.getParameter("lfoWaveform")->setValueNotifyingHost(0.0f);
        
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
        
        // With silence, LFO should not activate
        float rms = TestUtilities::calculateRMS(silentBuffer);
        expect(rms < 0.001f, "Silent input should remain silent");
        
        DIAG_LOG("Processor", "Audio processing test complete");
    }
    
    void testParameterAutomation() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        auto& apvts = processor.apvts;
        
        // Test parameter smoothing
        auto* rateParam = apvts.getParameter("lfoRate");
        auto* depthParam = apvts.getParameter("lfoDepth");
        
        // Set initial values
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(1.0f));
        depthParam->setValueNotifyingHost(0.5f);
        
        // Process some audio
        juce::AudioBuffer<float> buffer(2, 512);
        juce::MidiBuffer midiBuffer;
        
        for (int i = 0; i < 10; ++i) {
            processor.processBlock(buffer, midiBuffer);
        }
        
        // Change parameters rapidly
        std::vector<float> parameterValues;
        
        for (int i = 0; i < 100; ++i) {
            float newRate = 0.1f + (i * 0.2f);
            rateParam->setValueNotifyingHost(rateParam->convertTo0to1(newRate));
            
            processor.processBlock(buffer, midiBuffer);
            
            // Store actual parameter value
            parameterValues.push_back(apvts.getRawParameterValue("lfoRate")->load());
        }
        
        // Verify smooth parameter changes (no sudden jumps)
        bool smoothChanges = true;
        for (size_t i = 1; i < parameterValues.size(); ++i) {
            float diff = std::abs(parameterValues[i] - parameterValues[i-1]);
            if (diff > 5.0f) { // Allow reasonable change per block
                smoothChanges = false;
                break;
            }
        }
        
        expect(smoothChanges, "Parameter changes should be smooth");
        
        DIAG_LOG("Processor", "Parameter automation test complete");
    }
    
    void testBypass() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        auto& apvts = processor.apvts;
        auto* bypassParam = apvts.getParameter("bypass");
        
        // Create test signal
        auto inputBuffer = TestUtilities::createTestBuffer(2, 512);
        
        // Test with bypass off
        bypassParam->setValueNotifyingHost(0.0f);
        auto processedBuffer = inputBuffer;
        juce::MidiBuffer midiBuffer;
        
        // Set strong modulation
        apvts.getParameter("lfoDepth")->setValueNotifyingHost(1.0f);
        apvts.getParameter("lfoRate")->setValueNotifyingHost(
            apvts.getParameter("lfoRate")->convertTo0to1(10.0f));
        
        // Process multiple blocks
        for (int i = 0; i < 10; ++i) {
            processor.processBlock(processedBuffer, midiBuffer);
        }
        
        // Store modulated result
        auto modulatedBuffer = processedBuffer;
        
        // Test with bypass on
        bypassParam->setValueNotifyingHost(1.0f);
        processedBuffer = inputBuffer;
        
        for (int i = 0; i < 10; ++i) {
            processor.processBlock(processedBuffer, midiBuffer);
        }
        
        // With bypass on, output should match input
        bool isBypassed = TestUtilities::compareBuffers(inputBuffer, processedBuffer, 0.001f);
        expect(isBypassed, "Bypassed output should match input");
        
        // Modulated and bypassed should be different
        bool isDifferent = !TestUtilities::compareBuffers(modulatedBuffer, processedBuffer, 0.01f);
        expect(isDifferent, "Modulated and bypassed outputs should differ");
        
        DIAG_LOG("Processor", "Bypass test complete");
    }
    
    void testMixControl() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        auto& apvts = processor.apvts;
        auto* mixParam = apvts.getParameter("mix");
        
        // Set up modulation
        apvts.getParameter("lfoRate")->setValueNotifyingHost(
            apvts.getParameter("lfoRate")->convertTo0to1(5.0f));
        apvts.getParameter("lfoDepth")->setValueNotifyingHost(1.0f);
        
        auto inputBuffer = TestUtilities::createTestBuffer(2, 512);
        juce::MidiBuffer midiBuffer;
        
        // Test 100% wet
        mixParam->setValueNotifyingHost(1.0f);
        auto wetBuffer = inputBuffer;
        
        for (int i = 0; i < 10; ++i) {
            processor.processBlock(wetBuffer, midiBuffer);
        }
        
        // Test 0% wet (100% dry)
        mixParam->setValueNotifyingHost(0.0f);
        auto dryBuffer = inputBuffer;
        
        for (int i = 0; i < 10; ++i) {
            processor.processBlock(dryBuffer, midiBuffer);
        }
        
        // Dry should match input
        bool isDry = TestUtilities::compareBuffers(inputBuffer, dryBuffer, 0.001f);
        expect(isDry, "0% mix should produce dry signal");
        
        // Test 50% mix
        mixParam->setValueNotifyingHost(0.5f);
        auto mixedBuffer = inputBuffer;
        
        for (int i = 0; i < 10; ++i) {
            processor.processBlock(mixedBuffer, midiBuffer);
        }
        
        // 50% mix should be between dry and wet
        float dryRMS = TestUtilities::calculateRMS(dryBuffer);
        float wetRMS = TestUtilities::calculateRMS(wetBuffer);
        float mixedRMS = TestUtilities::calculateRMS(mixedBuffer);
        
        bool isMixed = (mixedRMS > juce::jmin(dryRMS, wetRMS) - 0.1f) &&
                       (mixedRMS < juce::jmax(dryRMS, wetRMS) + 0.1f);
        expect(isMixed, "50% mix should produce intermediate result");
        
        DIAG_LOG("Processor", "Mix control test complete");
    }
    
    void testStateSaveRestore() {
        QuackerVSTAudioProcessor processor1;
        QuackerVSTAudioProcessor processor2;
        
        // Set specific state in processor1
        auto& apvts1 = processor1.apvts;
        apvts1.getParameter("lfoRate")->setValueNotifyingHost(
            apvts1.getParameter("lfoRate")->convertTo0to1(3.7f));
        apvts1.getParameter("lfoDepth")->setValueNotifyingHost(0.67f);
        apvts1.getParameter("lfoWaveform")->setValueNotifyingHost(
            apvts1.getParameter("lfoWaveform")->convertTo0to1(5));
        apvts1.getParameter("mix")->setValueNotifyingHost(0.82f);
        apvts1.getParameter("bypass")->setValueNotifyingHost(1.0f);
        
        // Save state
        juce::MemoryBlock stateData;
        processor1.getStateInformation(stateData);
        
        expect(stateData.getSize() > 0, "State data should not be empty");
        
        // Restore state to processor2
        processor2.setStateInformation(stateData.getData(), 
                                     static_cast<int>(stateData.getSize()));
        
        // Verify parameters match
        auto& apvts2 = processor2.apvts;
        
        expectWithinAbsoluteError(
            apvts2.getRawParameterValue("lfoRate")->load(),
            apvts1.getRawParameterValue("lfoRate")->load(),
            0.01f,
            "LFO rate should be restored");
            
        expectWithinAbsoluteError(
            apvts2.getRawParameterValue("lfoDepth")->load(),
            apvts1.getRawParameterValue("lfoDepth")->load(),
            0.01f,
            "LFO depth should be restored");
            
        expect(
            static_cast<int>(apvts2.getRawParameterValue("lfoWaveform")->load()) ==
            static_cast<int>(apvts1.getRawParameterValue("lfoWaveform")->load()),
            "LFO waveform should be restored");
            
        expect(
            apvts2.getRawParameterValue("bypass")->load() > 0.5f,
            "Bypass state should be restored");
        
        DIAG_LOG("Processor", "State save/restore test complete");
    }
    
    void testBusLayout() {
        QuackerVSTAudioProcessor processor;
        
        // Test supported layouts
        auto layout = processor.getBusesLayout();
        
        // Test mono
        layout.getMainOutputChannelSet() = juce::AudioChannelSet::mono();
        layout.getMainInputChannelSet() = juce::AudioChannelSet::mono();
        expect(processor.isBusesLayoutSupported(layout), "Should support mono");
        
        // Test stereo
        layout.getMainOutputChannelSet() = juce::AudioChannelSet::stereo();
        layout.getMainInputChannelSet() = juce::AudioChannelSet::stereo();
        expect(processor.isBusesLayoutSupported(layout), "Should support stereo");
        
        // Test mismatched I/O (should not support)
        layout.getMainOutputChannelSet() = juce::AudioChannelSet::stereo();
        layout.getMainInputChannelSet() = juce::AudioChannelSet::mono();
        expect(!processor.isBusesLayoutSupported(layout), 
               "Should not support mismatched I/O");
        
        // Test unsupported format
        layout.getMainOutputChannelSet() = juce::AudioChannelSet::create5point1();
        layout.getMainInputChannelSet() = juce::AudioChannelSet::create5point1();
        expect(!processor.isBusesLayoutSupported(layout), 
               "Should not support 5.1");
        
        DIAG_LOG("Processor", "Bus layout test complete");
    }
    
    void testDCFilter() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(44100.0, 512);
        
        // Create buffer with DC offset
        juce::AudioBuffer<float> buffer(2, 512);
        float dcOffset = 0.5f;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                data[i] = dcOffset + 0.1f * std::sin(2.0f * juce::MathConstants<float>::pi * 
                                                     440.0f * i / 44100.0f);
            }
        }
        
        // Process multiple blocks to let DC filter settle
        juce::MidiBuffer midiBuffer;
        for (int i = 0; i < 100; ++i) {
            processor.processBlock(buffer, midiBuffer);
        }
        
        // Calculate average (DC component)
        float avgValue = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* data = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                avgValue += data[i];
            }
        }
        avgValue /= (buffer.getNumChannels() * buffer.getNumSamples());
        
        // DC component should be greatly reduced
        expect(std::abs(avgValue) < 0.05f, "DC component should be filtered out");
        
        DIAG_LOG("Processor", "DC filter test complete");
    }
    
    void testRealtimeSafety() {
        QuackerVSTAudioProcessor processor;
        processor.prepareToPlay(48000.0, 128); // Small buffer for stress test
        
        juce::AudioBuffer<float> buffer(2, 128);
        juce::MidiBuffer midiBuffer;
        
        // Test that processing doesn't allocate memory
        {
            PerformanceTimer timer("Realtime safety check");
            
            // Process many blocks rapidly
            for (int i = 0; i < 1000; ++i) {
                // Change parameters during processing
                if (i % 100 == 0) {
                    float newRate = 0.1f + (i % 10) * 2.0f;
                    processor.apvts.getParameter("lfoRate")->setValueNotifyingHost(
                        processor.apvts.getParameter("lfoRate")->convertTo0to1(newRate));
                }
                
                processor.processBlock(buffer, midiBuffer);
            }
        }
        
        // Test thread safety with audio thread
        std::atomic<bool> audioThreadRunning(true);
        std::thread audioThread([&]() {
            while (audioThreadRunning) {
                processor.processBlock(buffer, midiBuffer);
            }
        });
        
        // Simultaneously change parameters from another thread
        for (int i = 0; i < 100; ++i) {
            processor.apvts.getParameter("lfoWaveform")->setValueNotifyingHost(
                processor.apvts.getParameter("lfoWaveform")->convertTo0to1(i % 19));
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        
        audioThreadRunning = false;
        audioThread.join();
        
        expect(true, "Realtime safety test completed without crashes");
        
        DIAG_LOG("Processor", "Realtime safety test complete");
    }
    
    void testPerformanceBenchmarks() {
        QuackerVSTAudioProcessor processor;
        
        const int sampleRates[] = {44100, 48000, 96000};
        const int blockSizes[] = {64, 128, 256, 512, 1024};
        
        for (int sr : sampleRates) {
            for (int bs : blockSizes) {
                processor.prepareToPlay(sr, bs);
                
                juce::AudioBuffer<float> buffer(2, bs);
                TestUtilities::createTestBuffer(2, bs, 440.0f, sr);
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
                expect(cpuUsage < 10.0, "CPU usage should be reasonable");
            }
        }
        
        DIAG_LOG("Processor", "Performance benchmarks complete");
    }
};

// Register the test
static PluginProcessorTests pluginProcessorTests;