/*
  ==============================================================================

    QuackerVSTTests.h
    Main test header for QuackerVST plugin unit tests
    
  ==============================================================================
*/

#pragma once

// Check if we're using JUCE's CMake (which defines JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED)
// or Projucer (which creates JuceHeader.h)
#if __has_include(<JuceHeader.h>)
    #include <JuceHeader.h>
#else
    // Include JUCE modules directly when using CMake
    #include <juce_audio_basics/juce_audio_basics.h>
    #include <juce_audio_devices/juce_audio_devices.h>
    #include <juce_audio_formats/juce_audio_formats.h>
    #include <juce_audio_processors/juce_audio_processors.h>
    #include <juce_audio_utils/juce_audio_utils.h>
    #include <juce_core/juce_core.h>
    #include <juce_data_structures/juce_data_structures.h>
    #include <juce_dsp/juce_dsp.h>
    #include <juce_events/juce_events.h>
    #include <juce_graphics/juce_graphics.h>
    #include <juce_gui_basics/juce_gui_basics.h>
    #include <juce_gui_extra/juce_gui_extra.h>
#endif

// Plugin headers - adjust paths based on your setup
#include "../QuackerVST/Source/PluginProcessor.h"
#include "../QuackerVST/Source/TremoloLFO.h"
#include "../QuackerVST/Source/PresetManager.h"
#include "../QuackerVST/Source/WaveshapeLFO.h"
#include <chrono>
#include <fstream>
#include <sstream>

// Diagnostic logger for release builds
class DiagnosticLogger {
public:
    static DiagnosticLogger& getInstance() {
        static DiagnosticLogger instance;
        return instance;
    }
    
    void log(const juce::String& category, const juce::String& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::lock_guard<std::mutex> lock(mutex);
        
        // Console output
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                  << "] [" << category << "] " << message << std::endl;
        
        // File output
        if (logFile.is_open()) {
            logFile << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                    << "] [" << category << "] " << message << std::endl;
            logFile.flush();
        }
        
        // Store in memory for test assertions
        logEntries.push_back({category.toStdString(), message.toStdString()});
    }
    
    void startLogging(const juce::String& filename) {
        std::lock_guard<std::mutex> lock(mutex);
        if (logFile.is_open()) {
            logFile.close();
        }
        logFile.open(filename.toStdString(), std::ios::out | std::ios::app);
    }
    
    void stopLogging() {
        std::lock_guard<std::mutex> lock(mutex);
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    
    std::vector<std::pair<std::string, std::string>> getLogEntries() const {
        std::lock_guard<std::mutex> lock(mutex);
        return logEntries;
    }
    
    void clearLog() {
        std::lock_guard<std::mutex> lock(mutex);
        logEntries.clear();
    }
    
private:
    DiagnosticLogger() = default;
    ~DiagnosticLogger() {
        stopLogging();
    }
    
    mutable std::mutex mutex;
    std::ofstream logFile;
    std::vector<std::pair<std::string, std::string>> logEntries;
};

// Macro for diagnostic logging
#if JUCE_DEBUG
    #define DIAG_LOG(category, message) DiagnosticLogger::getInstance().log(category, message)
#else
    #define DIAG_LOG(category, message) ((void)0)
#endif

// Performance timer for benchmarking
class PerformanceTimer {
public:
    PerformanceTimer(const juce::String& name) : testName(name) {
        start = std::chrono::high_resolution_clock::now();
    }
    
    ~PerformanceTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        DIAG_LOG("Performance", testName + " took " + juce::String(duration.count()) + " µs");
    }
    
private:
    juce::String testName;
    std::chrono::high_resolution_clock::time_point start;
};

// Test utilities
class TestUtilities {
public:
    // Generate test audio buffer
    static juce::AudioBuffer<float> createTestBuffer(int numChannels, int numSamples, float frequency = 440.0f, double sampleRate = 44100.0) {
        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        
        for (int channel = 0; channel < numChannels; ++channel) {
            auto* channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < numSamples; ++sample) {
                float phase = (float)sample / (float)sampleRate * frequency * 2.0f * juce::MathConstants<float>::pi;
                channelData[sample] = std::sin(phase);
            }
        }
        
        return buffer;
    }
    
    // Check if buffer contains valid audio (not silence, not clipping)
    static bool isValidAudioBuffer(const juce::AudioBuffer<float>& buffer, float tolerance = 0.0001f) {
        bool hasSignal = false;
        bool hasClipping = false;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* data = buffer.getReadPointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                float value = data[sample];
                
                if (std::abs(value) > tolerance) {
                    hasSignal = true;
                }
                
                if (std::abs(value) > 1.0f) {
                    hasClipping = true;
                }
                
                if (!std::isfinite(value)) {
                    return false; // NaN or infinity
                }
            }
        }
        
        return hasSignal && !hasClipping;
    }
    
    // Calculate RMS of buffer
    static float calculateRMS(const juce::AudioBuffer<float>& buffer) {
        float sum = 0.0f;
        int totalSamples = 0;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* data = buffer.getReadPointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                sum += data[sample] * data[sample];
                totalSamples++;
            }
        }
        
        return totalSamples > 0 ? std::sqrt(sum / totalSamples) : 0.0f;
    }
    
    // Compare two buffers
    static bool compareBuffers(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b, float tolerance = 0.0001f) {
        if (a.getNumChannels() != b.getNumChannels() || a.getNumSamples() != b.getNumSamples()) {
            return false;
        }
        
        for (int channel = 0; channel < a.getNumChannels(); ++channel) {
            auto* dataA = a.getReadPointer(channel);
            auto* dataB = b.getReadPointer(channel);
            
            for (int sample = 0; sample < a.getNumSamples(); ++sample) {
                if (std::abs(dataA[sample] - dataB[sample]) > tolerance) {
                    return false;
                }
            }
        }
        
        return true;
    }
};

// Base test class with common functionality
class QuackerTestBase : public juce::UnitTest {
public:
    QuackerTestBase(const juce::String& name) : UnitTest(name) {}
    
protected:
    void logTestStart(const juce::String& testName) {
        DIAG_LOG("Test", "Starting: " + testName);
    }
    
    void logTestEnd(const juce::String& testName, bool passed) {
        DIAG_LOG("Test", testName + (passed ? " PASSED" : " FAILED"));
    }
    
    // Helper to run a test with logging
    void runSubTest(const juce::String& name, std::function<void()> test) {
        logTestStart(name);
        bool passed = true;
        
        try {
            test();
        } catch (const std::exception& e) {
            passed = false;
            expect(false, name + " threw exception: " + juce::String(e.what()));
        }
        
        logTestEnd(name, passed);
    }
    
    // Thread safety test helper
    void testThreadSafety(const juce::String& testName, std::function<void()> operation, int numThreads = 4) {
        std::vector<std::thread> threads;
        std::atomic<int> completedThreads(0);
        std::atomic<bool> hasError(false);
        
        DIAG_LOG("ThreadSafety", "Starting " + testName + " with " + juce::String(numThreads) + " threads");
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([&]() {  // Remove unused capture 'i'
                try {
                    for (int j = 0; j < 100; ++j) {
                        operation();
                    }
                    completedThreads++;
                } catch (...) {
                    hasError = true;
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        expect(!hasError, testName + " - No exceptions in threads");
        expect(completedThreads == numThreads, testName + " - All threads completed");
    }
};