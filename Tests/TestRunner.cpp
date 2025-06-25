/*
  ==============================================================================

    TestRunner.cpp
    Main test runner executable for QuackerVST tests
    
  ==============================================================================
*/

// Include JUCE header first
#include <JuceHeader.h>

// Then include our test framework
#include "QuackerVSTTests.h"

// Include test implementations - these will register themselves
#include "TremoloLFOTests.cpp"
#include "PresetManagerTests.cpp"
#include "PluginProcessorTests.cpp"

//==============================================================================
class TestRunnerApplication : public juce::JUCEApplication
{
public:
    TestRunnerApplication() {}

    const juce::String getApplicationName() override { return "QuackerVST Test Runner"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        // Parse command line arguments
        juce::StringArray args;
        args.addTokens(commandLine, true);
        
        bool generateReport = args.contains("--report");
        bool verboseMode = args.contains("--verbose");
        juce::String outputFile = "test_results.xml";
        
        // Check for custom output file
        int outputIndex = args.indexOf("--output");
        if (outputIndex >= 0 && outputIndex < args.size() - 1) {
            outputFile = args[outputIndex + 1];
        }
        
        // Initialize diagnostic logger
        if (verboseMode) {
            DiagnosticLogger::getInstance().startLogging("test_diagnostics.log");
        }
        
        // Create test runner
        juce::UnitTestRunner testRunner;
        
        // Set up console logging
        if (verboseMode) {
            std::cout << "Running in verbose mode..." << std::endl;
        }
        
        testRunner.setAssertOnFailure(false);
        
        // Run all tests
        std::cout << "Running QuackerVST Unit Tests..." << std::endl;
        std::cout << "================================" << std::endl;
        
        testRunner.runAllTests();
        
        // Get results
        int numTests = testRunner.getNumResults();
        int numPassed = 0;
        int numFailed = 0;
        
        for (int i = 0; i < numTests; ++i) {
            auto* result = testRunner.getResult(i);
            if (result != nullptr) {
                if (result->failures > 0) {
                    numFailed++;
                } else {
                    numPassed++;
                }
                
                // Print result
                std::cout << result->unitTestName << ": " 
                         << (result->failures > 0 ? "FAILED" : "PASSED")
                         << " (" << result->passes << " passes, " 
                         << result->failures << " failures)" << std::endl;
                
                // Print failure messages
                if (verboseMode || result->failures > 0) {
                    for (const auto& message : result->messages) {
                        std::cout << "  - " << message << std::endl;
                    }
                }
            }
        }
        
        std::cout << "================================" << std::endl;
        std::cout << "Total: " << numTests << " tests" << std::endl;
        std::cout << "Passed: " << numPassed << std::endl;
        std::cout << "Failed: " << numFailed << std::endl;
        
        // Generate XML report if requested
        if (generateReport) {
            generateXMLReport(testRunner, outputFile);
        }
        
        // Stop logging
        DiagnosticLogger::getInstance().stopLogging();
        
        // Set exit code based on results
        setApplicationReturnValue(numFailed > 0 ? 1 : 0);
        quit();
    }

    void shutdown() override {}

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    void generateXMLReport(juce::UnitTestRunner& runner, const juce::String& filename)
    {
        juce::XmlElement xml("testsuites");
        xml.setAttribute("name", "QuackerVST Tests");
        xml.setAttribute("tests", runner.getNumResults());
        
        int totalFailures = 0;
        double totalTime = 0.0;
        
        auto* testsuite = xml.createNewChildElement("testsuite");
        testsuite->setAttribute("name", "QuackerVST");
        testsuite->setAttribute("tests", runner.getNumResults());
        
        for (int i = 0; i < runner.getNumResults(); ++i) {
            auto* result = runner.getResult(i);
            if (result != nullptr) {
                auto* testcase = testsuite->createNewChildElement("testcase");
                testcase->setAttribute("name", result->unitTestName);
                testcase->setAttribute("classname", "QuackerVST");
                testcase->setAttribute("time", "0.0"); // Would need timing info
                
                if (result->failures > 0) {
                    totalFailures += result->failures;
                    
                    for (const auto& message : result->messages) {
                        auto* failure = testcase->createNewChildElement("failure");
                        failure->setAttribute("message", message);
                        failure->setAttribute("type", "AssertionError");
                        failure->addTextElement(message);
                    }
                }
            }
        }
        
        testsuite->setAttribute("failures", totalFailures);
        testsuite->setAttribute("time", totalTime);
        
        // Write to file
        juce::File outputFile(filename);
        if (xml.writeTo(outputFile)) {
            std::cout << "Test report written to: " << filename << std::endl;
        } else {
            std::cerr << "Failed to write test report!" << std::endl;
        }
    }
};

//==============================================================================
START_JUCE_APPLICATION(TestRunnerApplication)