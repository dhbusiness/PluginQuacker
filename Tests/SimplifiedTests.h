/*
  ==============================================================================

    SimplifiedTests.h
    Simplified test framework for basic plugin testing
    
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <iostream>
#include <vector>
#include <functional>
#include <chrono>

// Only include plugin headers that are essential for testing
#ifdef __has_include
    #if __has_include("../QuackerVST/Source/PluginProcessor.h")
        #include "../QuackerVST/Source/PluginProcessor.h"
    #endif
    #if __has_include("../QuackerVST/Source/TremoloLFO.h")
        #include "../QuackerVST/Source/TremoloLFO.h"
    #endif
    #if __has_include("../QuackerVST/Source/PresetManager.h")
        #include "../QuackerVST/Source/PresetManager.h"
    #endif
#endif

// Simplified test framework
class SimpleTest {
public:
    SimpleTest(const juce::String& name) : testName(name), testsPassed(0), testsFailed(0) {}
    
    virtual ~SimpleTest() {}
    
    virtual void runTest() = 0;
    
    const juce::String& getName() const { return testName; }
    int getPassCount() const { return testsPassed; }
    int getFailCount() const { return testsFailed; }
    
protected:
    void expect(bool condition, const juce::String& message) {
        if (condition) {
            testsPassed++;
            std::cout << "  ✓ " << message << std::endl;
        } else {
            testsFailed++;
            std::cout << "  ✗ " << message << std::endl;
            failureMessages.push_back(message);
        }
    }
    
    void expectWithinRange(float actual, float expected, float tolerance, const juce::String& message) {
        float diff = std::abs(actual - expected);
        if (diff <= tolerance) {
            testsPassed++;
            std::cout << "  ✓ " << message << " (actual: " << actual << ", expected: " << expected << ")" << std::endl;
        } else {
            testsFailed++;
            std::cout << "  ✗ " << message << " (actual: " << actual << ", expected: " << expected << ", diff: " << diff << ")" << std::endl;
            failureMessages.push_back(message + " - values out of range");
        }
    }
    
    void logMessage(const juce::String& message) {
        std::cout << "  → " << message << std::endl;
    }
    
    juce::String testName;
    int testsPassed;
    int testsFailed;
    std::vector<juce::String> failureMessages;
};

// Test registry
class TestRegistry {
public:
    static TestRegistry& getInstance() {
        static TestRegistry instance;
        return instance;
    }
    
    void registerTest(SimpleTest* test) {
        tests.push_back(test);
    }
    
    void runAllTests() {
        int totalPassed = 0;
        int totalFailed = 0;
        
        std::cout << "Running " << tests.size() << " test suites..." << std::endl;
        std::cout << "======================================" << std::endl;
        
        for (auto* test : tests) {
            std::cout << "\nRunning: " << test->getName() << std::endl;
            std::cout << "------------------------------" << std::endl;
            
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                test->runTest();
            } catch (const std::exception& e) {
                std::cout << "  ✗ Test threw exception: " << e.what() << std::endl;
                test->testsFailed++;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            std::cout << "Result: " << test->getPassCount() << " passed, " 
                     << test->getFailCount() << " failed (" 
                     << duration.count() << "ms)" << std::endl;
            
            totalPassed += test->getPassCount();
            totalFailed += test->getFailCount();
        }
        
        std::cout << "\n======================================" << std::endl;
        std::cout << "TOTAL RESULTS:" << std::endl;
        std::cout << "Passed: " << totalPassed << std::endl;
        std::cout << "Failed: " << totalFailed << std::endl;
        std::cout << "Success rate: " << (totalPassed * 100 / std::max(1, totalPassed + totalFailed)) << "%" << std::endl;
        
        // Set exit code
        exitCode = (totalFailed > 0) ? 1 : 0;
    }
    
    int getExitCode() const { return exitCode; }
    
private:
    std::vector<SimpleTest*> tests;
    int exitCode = 0;
};

// Automatic test registration
template<typename TestClass>
class TestRegistrar {
public:
    TestRegistrar() {
        static TestClass instance;
        TestRegistry::getInstance().registerTest(&instance);
    }
};

#define REGISTER_TEST(TestClass) \
    static TestRegistrar<TestClass> TestClass##_registrar;

// Test utilities
class TestUtils {
public:
    static juce::AudioBuffer<float> createSineWave(int numChannels, int numSamples, float frequency = 440.0f, double sampleRate = 44100.0) {
        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        
        for (int channel = 0; channel < numChannels; ++channel) {
            auto* data = buffer.getWritePointer(channel);
            for (int sample = 0; sample < numSamples; ++sample) {
                float phase = (float)sample / (float)sampleRate * frequency * 2.0f * juce::MathConstants<float>::pi;
                data[sample] = 0.5f * std::sin(phase);
            }
        }
        
        return buffer;
    }
    
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
    
    static bool isValidAudio(const juce::AudioBuffer<float>& buffer) {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* data = buffer.getReadPointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                float value = data[sample];
                if (!std::isfinite(value) || std::abs(value) > 10.0f) {
                    return false;
                }
            }
        }
        return true;
    }
};