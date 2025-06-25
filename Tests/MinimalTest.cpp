/*
  ==============================================================================

    MinimalTest.cpp
    A minimal test to verify your test setup is working
    
  ==============================================================================
*/

#include <JuceHeader.h>

class MinimalTest : public juce::UnitTest {
public:
    MinimalTest() : UnitTest("Minimal Test") {}
    
    void runTest() override {
        beginTest("Basic Math Test");
        
        expect(2 + 2 == 4, "Basic addition should work");
        expect(10 * 5 == 50, "Basic multiplication should work");
        
        beginTest("JUCE Functionality Test");
        
        // Test JUCE string
        juce::String testString = "Hello, QuackerVST!";
        expect(testString.contains("Quacker"), "String should contain 'Quacker'");
        
        // Test JUCE random
        juce::Random random;
        int randomValue = random.nextInt(100);
        expect(randomValue >= 0 && randomValue < 100, "Random value should be in range");
        
        // Test audio buffer creation
        juce::AudioBuffer<float> buffer(2, 512);
        expect(buffer.getNumChannels() == 2, "Buffer should have 2 channels");
        expect(buffer.getNumSamples() == 512, "Buffer should have 512 samples");
        
        logMessage("All minimal tests passed!");
    }
};

// Register the test
static MinimalTest minimalTest;

// Simple main function for standalone testing
int main(int argc, char* argv[]) {
    juce::UnitTestRunner runner;
    runner.runAllTests();
    
    int numPassed = 0;
    int numFailed = 0;
    
    for (int i = 0; i < runner.getNumResults(); ++i) {
        auto* result = runner.getResult(i);
        if (result->failures > 0) {
            numFailed++;
        } else {
            numPassed++;
        }
    }
    
    std::cout << "\nTest Summary:" << std::endl;
    std::cout << "Passed: " << numPassed << std::endl;
    std::cout << "Failed: " << numFailed << std::endl;
    
    return numFailed > 0 ? 1 : 0;
}