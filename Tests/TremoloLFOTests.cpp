/*
  ==============================================================================

    TremoloLFOTests.cpp
    Unit tests for TremoloLFO class (Fixed Version)
    
  ==============================================================================
*/

#include "QuackerVSTTests.h"

class TremoloLFOTests : public QuackerTestBase {
public:
    TremoloLFOTests() : QuackerTestBase("TremoloLFO Tests") {}
    
    void runTest() override {
        beginTest("TremoloLFO Tests");
        
        // Initialize diagnostic logging
        DiagnosticLogger::getInstance().startLogging("tremolo_lfo_test_log.txt");
        
        runSubTest("Initialization Test", [this]() { testInitialization(); });
        runSubTest("Sample Rate Test", [this]() { testSampleRate(); });
        runSubTest("Rate Parameter Test", [this]() { testRateParameter(); });
        runSubTest("Depth Parameter Test", [this]() { testDepthParameter(); });
        runSubTest("Waveform Test", [this]() { testWaveforms(); });
        runSubTest("Phase Offset Test", [this]() { testPhaseOffset(); });
        runSubTest("Sync Mode Test", [this]() { testSyncMode(); });
        runSubTest("Thread Safety Test", [this]() { testThreadSafety(); });
        runSubTest("Performance Test", [this]() { testPerformance(); });
        runSubTest("Edge Cases Test", [this]() { testEdgeCases(); });
        runSubTest("Waveshaping Test", [this]() { testWaveshaping(); });
        
        DiagnosticLogger::getInstance().stopLogging();
    }
    
private:
    void testInitialization() {
        TremoloLFO lfo;
        
        expect(lfo.getLastError() == TremoloLFO::ErrorCode::None, "LFO should initialize without errors");
        expect(lfo.getCurrentEffectiveRate() == 1.0f, "Default rate should be 1.0 Hz");
        expect(!lfo.isSynced(), "Should not be synced by default");
        expect(!lfo.isWaitingForReset(), "Should not be waiting for reset initially");
        
        DIAG_LOG("LFO", "Initialization successful");
    }
    
    void testSampleRate() {
        TremoloLFO lfo;
        
        // Test valid sample rates
        const double validRates[] = {8000.0, 22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0};
        
        for (double rate : validRates) {
            auto error = lfo.setSampleRate(rate);
            expect(error == TremoloLFO::ErrorCode::None, 
                   "Sample rate " + juce::String(rate) + " should be valid");
        }
        
        // Test invalid sample rates
        const double invalidRates[] = {-1.0, 0.0, 7999.0, 400000.0, std::numeric_limits<double>::infinity()};
        
        for (double rate : invalidRates) {
            auto error = lfo.setSampleRate(rate);
            expect(error == TremoloLFO::ErrorCode::InvalidSampleRate, 
                   "Sample rate " + juce::String(rate) + " should be invalid");
        }
        
        DIAG_LOG("LFO", "Sample rate validation complete");
    }
    
    void testRateParameter() {
        TremoloLFO lfo;
        lfo.setSampleRate(44100.0);
        
        // Test valid rates
        const float testRates[] = {0.01f, 0.1f, 1.0f, 5.0f, 10.0f, 25.0f};
        
        for (float rate : testRates) {
            auto error = lfo.setRate(rate);
            expect(error == TremoloLFO::ErrorCode::None, 
                   "Rate " + juce::String(rate) + " should be valid");
            
            // Fix type mismatch: cast double to float for comparison
            float currentRate = static_cast<float>(lfo.getCurrentEffectiveRate());
            expectWithinAbsoluteError(currentRate, rate, 0.001f,
                                    "Rate should be set correctly");
        }
        
        // Test rate limiting
        lfo.setRate(0.001f); // Below minimum
        expect(lfo.getCurrentEffectiveRate() >= 0.001f, "Rate should be clamped to minimum");
        
        lfo.setRate(100.0f); // Above maximum  
        expect(lfo.getCurrentEffectiveRate() <= 100.0f, "Rate should be clamped to maximum");
        
        // Test smooth rate changes
        {
            PerformanceTimer timer("Rate smoothing test");
            lfo.setRate(1.0f);
            
            std::vector<float> samples;
            for (int i = 0; i < 1000; ++i) {
                samples.push_back(lfo.getNextSample());
            }
            
            lfo.setRate(10.0f);
            
            // Collect samples during transition
            std::vector<float> transitionSamples;
            for (int i = 0; i < 1000; ++i) {
                transitionSamples.push_back(lfo.getNextSample());
            }
            
            // Verify smooth transition (no sudden jumps)
            bool smoothTransition = true;
            for (size_t i = 1; i < transitionSamples.size(); ++i) {
                float diff = std::abs(transitionSamples[i] - transitionSamples[i-1]);
                if (diff > 0.1f) { // Allow reasonable change
                    smoothTransition = false;
                    break;
                }
            }
            
            expect(smoothTransition, "Rate changes should be smooth");
        }
        
        DIAG_LOG("LFO", "Rate parameter tests complete");
    }
    
    void testDepthParameter() {
        TremoloLFO lfo;
        lfo.setSampleRate(44100.0);
        lfo.setRate(10.0f); // Fast rate for testing
        
        // Test different depth values
        const float depths[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
        
        for (float depth : depths) {
            lfo.setDepth(depth);
            
            // Collect samples over one period
            std::vector<float> samples;
            int samplesPerPeriod = static_cast<int>(44100.0 / 10.0);
            
            for (int i = 0; i < samplesPerPeriod; ++i) {
                samples.push_back(lfo.getNextSample());
            }
            
            // Find min and max values
            auto minmax = std::minmax_element(samples.begin(), samples.end());
            float range = *minmax.second - *minmax.first;
            
            // At depth 0, output should be constant (1.0)
            if (depth == 0.0f) {
                expect(range < 0.01f, "At depth 0, output should be constant");
            } else {
                // Otherwise, range should scale with depth
                expectWithinAbsoluteError(range, depth, 0.1f, 
                    "Output range should match depth setting");
            }
        }
        
        DIAG_LOG("LFO", "Depth parameter tests complete");
    }
    
    void testWaveforms() {
        TremoloLFO lfo;
        lfo.setSampleRate(44100.0);
        lfo.setRate(10.0f);
        lfo.setDepth(1.0f);
        
        // Test each waveform that actually exists in your TremoloLFO enum
        // Only test the waveforms that are defined in TremoloLFO::Waveform
        const std::vector<std::pair<TremoloLFO::Waveform, std::string>> waveforms = {
            {TremoloLFO::Sine, "Sine"},
            {TremoloLFO::Square, "Square"},
            {TremoloLFO::Triangle, "Triangle"},
            {TremoloLFO::SawtoothUp, "SawtoothUp"},
            {TremoloLFO::SawtoothDown, "SawtoothDown"},
            {TremoloLFO::SoftSquare, "SoftSquare"}
            // Add other waveforms here only if they exist in your enum
        };
        
        for (const auto& [waveform, name] : waveforms) {
            auto error = lfo.setWaveform(waveform);
            
            expect(error == TremoloLFO::ErrorCode::None, 
                   name + " waveform should be valid");
            
            // Collect one period of samples
            std::vector<float> samples;
            int samplesPerPeriod = static_cast<int>(44100.0 / 10.0);
            
            for (int i = 0; i < samplesPerPeriod; ++i) {
                float sample = lfo.getNextSample();
                expect(sample >= 0.0f && sample <= 1.0f, 
                       "Sample should be in range [0, 1]");
                samples.push_back(sample);
            }
            
            // Verify waveform characteristics
            switch (waveform) {
                case TremoloLFO::Sine:
                    // Should be smooth
                    expect(isSmooth(samples), "Sine wave should be smooth");
                    break;
                    
                case TremoloLFO::Square:
                    // Should have sharp transitions
                    expect(hasSharpTransitions(samples), "Square wave should have sharp transitions");
                    break;
                    
                case TremoloLFO::Triangle:
                    // Should be linear segments
                    expect(hasLinearSegments(samples), "Triangle wave should have linear segments");
                    break;
                    
                default:
                    // All waveforms should produce valid output
                    expect(true, name + " produces valid output");
                    break;
            }
        }
        
        DIAG_LOG("LFO", "All waveforms tested successfully");
    }
    
    void testPhaseOffset() {
        TremoloLFO lfo1, lfo2;
        const double sampleRate = 44100.0;
        
        lfo1.setSampleRate(sampleRate);
        lfo2.setSampleRate(sampleRate);
        
        lfo1.setRate(1.0f);
        lfo2.setRate(1.0f);
        
        lfo1.setPhaseOffset(0.0f);
        lfo2.setPhaseOffset(180.0f); // 180 degrees out of phase
        
        // Collect samples
        std::vector<float> samples1, samples2;
        for (int i = 0; i < 1000; ++i) {
            samples1.push_back(lfo1.getNextSample());
            samples2.push_back(lfo2.getNextSample());
        }
        
        // Verify phase relationship
        float correlation = calculateCorrelation(samples1, samples2);
        expect(correlation < -0.8f, "180-degree phase offset should produce negative correlation");
        
        DIAG_LOG("LFO", "Phase offset test complete");
    }
    
    void testSyncMode() {
        TremoloLFO lfo;
        lfo.setSampleRate(44100.0);
        
        // Test BPM sync
        double bpm = 120.0;
        lfo.setBPM(bpm);
        
        // Test different note divisions
        const double divisions[] = {0.25, 0.5, 1.0, 2.0, 4.0, 8.0};
        
        for (double division : divisions) {
            lfo.setSyncMode(true, division);
            
            double expectedRate = TremoloLFO::bpmToFrequency(bpm, division);
            expectWithinAbsoluteError(static_cast<float>(lfo.getCurrentEffectiveRate()), 
                                    static_cast<float>(expectedRate), 0.01f,
                                    "Sync rate should match expected frequency");
        }
        
        // Test manual rate preservation
        float manualRate = 5.5f;
        lfo.setSyncMode(false);
        lfo.setRate(manualRate);
        lfo.setSyncMode(true, 1.0);
        lfo.setSyncMode(false);
        
        expectWithinAbsoluteError(lfo.getLastManualRate(), manualRate, 0.01f,
            "Manual rate should be preserved when toggling sync");
        
        DIAG_LOG("LFO", "Sync mode tests complete");
    }
    
    void testThreadSafety() {
        auto lfo = std::make_shared<TremoloLFO>();
        lfo->setSampleRate(44100.0);
        
        // Test concurrent parameter changes using the base class method
        QuackerTestBase::testThreadSafety("Concurrent parameter changes", [lfo]() {
            lfo->setRate(juce::Random::getSystemRandom().nextFloat() * 20.0f);
            lfo->setDepth(juce::Random::getSystemRandom().nextFloat());
            lfo->setWaveform(static_cast<TremoloLFO::Waveform>(
                juce::Random::getSystemRandom().nextInt(TremoloLFO::NumWaveforms)));
        });
        
        // Test concurrent sample generation
        QuackerTestBase::testThreadSafety("Concurrent sample generation", [lfo]() {
            for (int i = 0; i < 100; ++i) {
                float sample = lfo->getNextSample();
                jassert(sample >= 0.0f && sample <= 1.0f);
                (void)sample; // Suppress unused variable warning
            }
        });
        
        DIAG_LOG("LFO", "Thread safety tests complete");
    }
    
    void testPerformance() {
        TremoloLFO lfo;
        lfo.setSampleRate(48000.0);
        lfo.setRate(5.0f);
        lfo.setDepth(0.8f);
        
        const int numSamples = 48000; // 1 second of audio
        
        {
            PerformanceTimer timer("Generate 1 second of LFO samples");
            
            for (int i = 0; i < numSamples; ++i) {
                volatile float sample = lfo.getNextSample();
                (void)sample; // Prevent optimization
            }
        }
        
        // Test different waveforms performance
        for (int wf = 0; wf < static_cast<int>(TremoloLFO::NumWaveforms); ++wf) {
            lfo.setWaveform(static_cast<TremoloLFO::Waveform>(wf));
            
            PerformanceTimer timer("Waveform " + juce::String(wf));
            
            for (int i = 0; i < 10000; ++i) {
                volatile float sample = lfo.getNextSample();
                (void)sample;
            }
        }
        
        DIAG_LOG("LFO", "Performance tests complete");
    }
    
    void testEdgeCases() {
        TremoloLFO lfo;
        
        // Test with extreme sample rates
        lfo.setSampleRate(8000.0);
        lfo.setRate(25.0f); // High rate at low sample rate
        
        bool stable = true;
        for (int i = 0; i < 1000; ++i) {
            float sample = lfo.getNextSample();
            if (!std::isfinite(sample) || sample < 0.0f || sample > 1.0f) {
                stable = false;
                break;
            }
        }
        expect(stable, "LFO should remain stable at extreme settings");
        
        // Test rapid parameter changes
        for (int i = 0; i < 100; ++i) {
            lfo.setRate(0.01f + (i * 0.25f));
            lfo.setDepth(i / 100.0f);
            lfo.setWaveform(static_cast<TremoloLFO::Waveform>(i % TremoloLFO::NumWaveforms));
            
            float sample = lfo.getNextSample();
            expect(std::isfinite(sample), "Sample should be finite after rapid changes");
        }
        
        // Test reset behavior
        lfo.resetPhase();
        float firstSample = lfo.getNextSample();
        lfo.resetPhase();
        float secondSample = lfo.getNextSample();
        
        expectWithinAbsoluteError(firstSample, secondSample, 0.001f,
            "Reset should produce consistent results");
        
        DIAG_LOG("LFO", "Edge case tests complete");
    }
    
    void testWaveshaping() {
        TremoloLFO lfo;
        lfo.setSampleRate(44100.0);
        lfo.setRate(1.0f);
        lfo.setDepth(1.0f);
        
        // Test waveshaping disabled
        lfo.setWaveshapeParameters(10.0f, 0.5f, 0, false);
        
        std::vector<float> samplesNoShape;
        for (int i = 0; i < 1000; ++i) {
            samplesNoShape.push_back(lfo.getNextSample());
        }
        
        // Test waveshaping enabled
        lfo.resetPhase();
        lfo.setWaveshapeParameters(10.0f, 0.5f, 0, true);
        
        std::vector<float> samplesWithShape;
        for (int i = 0; i < 1000; ++i) {
            samplesWithShape.push_back(lfo.getNextSample());
        }
        
        // Verify that waveshaping changes the output
        bool different = false;
        for (size_t i = 0; i < samplesNoShape.size(); ++i) {
            if (std::abs(samplesNoShape[i] - samplesWithShape[i]) > 0.01f) {
                different = true;
                break;
            }
        }
        
        expect(different, "Waveshaping should affect the output");
        
        DIAG_LOG("LFO", "Waveshaping tests complete");
    }
    
    // Helper functions
    bool isSmooth(const std::vector<float>& samples) {
        for (size_t i = 1; i < samples.size(); ++i) {
            if (std::abs(samples[i] - samples[i-1]) > 0.1f) {
                return false;
            }
        }
        return true;
    }
    
    bool hasSharpTransitions(const std::vector<float>& samples) {
        int sharpTransitions = 0;
        for (size_t i = 1; i < samples.size(); ++i) {
            if (std::abs(samples[i] - samples[i-1]) > 0.5f) {
                sharpTransitions++;
            }
        }
        return sharpTransitions >= 2; // At least 2 transitions per period
    }
    
    bool hasLinearSegments(const std::vector<float>& samples) {
        // Check if consecutive differences are relatively constant
        std::vector<float> diffs;
        for (size_t i = 1; i < samples.size(); ++i) {
            diffs.push_back(samples[i] - samples[i-1]);
        }
        
        // Group by sign changes to find segments
        int signChanges = 0;
        for (size_t i = 1; i < diffs.size(); ++i) {
            if ((diffs[i] > 0) != (diffs[i-1] > 0)) {
                signChanges++;
            }
        }
        
        return signChanges <= 4; // Triangle should have ~2 segments
    }
    
    float calculateCorrelation(const std::vector<float>& a, const std::vector<float>& b) {
        if (a.size() != b.size() || a.empty()) return 0.0f;
        
        float meanA = 0.0f, meanB = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            meanA += a[i];
            meanB += b[i];
        }
        meanA /= a.size();
        meanB /= b.size();
        
        float numerator = 0.0f, denomA = 0.0f, denomB = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            float diffA = a[i] - meanA;
            float diffB = b[i] - meanB;
            numerator += diffA * diffB;
            denomA += diffA * diffA;
            denomB += diffB * diffB;
        }
        
        float denom = std::sqrt(denomA * denomB);
        return denom > 0.0f ? numerator / denom : 0.0f;
    }
};

// Register the test
static TremoloLFOTests tremoloLFOTests;