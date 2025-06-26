/*
  ==============================================================================

    Main.cpp
    Created: Test Runner for TremoloViola Plugin
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SmokeTests.h"
#include "TremoloLFOTests.h"
#include "PresetManagerTests.h"
#include "PluginTests.h"

//==============================================================================
class TestApplication : public juce::JUCEApplication
{
public:
    TestApplication() = default;

    const juce::String getApplicationName() override { return "TremoloViola Tests"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        // Parse command line arguments
        juce::StringArray args = juce::StringArray::fromTokens(commandLine, true);
        
        bool runSmokeTests = args.contains("--smoke") || args.contains("--all") || args.isEmpty();
        bool runUnitTests = args.contains("--unit") || args.contains("--all") || args.isEmpty();
        bool verboseOutput = args.contains("--verbose") || args.contains("-v");
        
        if (args.contains("--help") || args.contains("-h"))
        {
            showHelp();
            quit();
            return;
        }

        std::cout << "=== TremoloViola Plugin Test Suite ===" << std::endl;
        std::cout << "Build: " << juce::Time::getCompilationDate().toString(true, true) << std::endl;
        std::cout << "JUCE Version: " << juce::SystemStats::getJUCEVersion() << std::endl;
        std::cout << std::endl;

        // Create and register test runners
        juce::Array<juce::UnitTest*> tests;
        
        if (runSmokeTests)
        {
            tests.add(&smokeTests);
        }
        
        if (runUnitTests)
        {
            tests.add(&lfoTests);
            tests.add(&presetTests);
            tests.add(&pluginTests);
        }

        if (tests.isEmpty())
        {
            std::cout << "No tests selected to run." << std::endl;
            quit();
            return;
        }

        // Run the tests
        TestRunner runner;
        runner.setVerbose(verboseOutput);
        
        int totalTests = 0;
        int totalPasses = 0;
        int totalFailures = 0;
        
        for (auto* test : tests)
        {
            std::cout << "\n--- Running " << test->getName() << " ---" << std::endl;
            
            auto result = runner.runTest(test);
            
            totalTests += result.tests;
            totalPasses += result.passes;
            totalFailures += result.failures;
            
            if (result.failures > 0)
            {
                std::cout << "❌ " << test->getName() << " - " 
                         << result.failures << " failures out of " << result.tests << " tests" << std::endl;
            }
            else
            {
                std::cout << "✅ " << test->getName() << " - All " 
                         << result.tests << " tests passed" << std::endl;
            }
        }

        // Print summary
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << totalPasses << std::endl;
        std::cout << "Failed: " << totalFailures << std::endl;
        
        if (totalFailures == 0)
        {
            std::cout << "🎉 All tests passed!" << std::endl;
            setApplicationReturnValue(0);
        }
        else
        {
            std::cout << "💥 " << totalFailures << " test(s) failed!" << std::endl;
            setApplicationReturnValue(1);
        }

        quit();
    }

    void shutdown() override
    {
        // Clean up if needed
    }

    void anotherInstanceStarted(const juce::String&) override
    {
        // Handle multiple instances if needed
    }

private:
    struct TestResult
    {
        int tests = 0;
        int passes = 0;
        int failures = 0;
    };

    class TestRunner : public juce::UnitTestRunner
    {
    public:
        TestResult runTest(juce::UnitTest* test)
        {
            TestResult result;
            
            // Reset counters
            testsPassed = 0;
            testsFailed = 0;
            
            // Run the test
            runTest(test, juce::Random().nextInt64());
            
            result.tests = testsPassed + testsFailed;
            result.passes = testsPassed;
            result.failures = testsFailed;
            
            return result;
        }
        
        void setVerbose(bool shouldBeVerbose)
        {
            verbose = shouldBeVerbose;
        }
        
    private:
        void logMessage(const juce::String& message) override
        {
            if (verbose)
                std::cout << "  " << message << std::endl;
        }
        
        bool verbose = false;
        int testsPassed = 0;
        int testsFailed = 0;
        
        // Override these to count results
        void beginNewTest(juce::UnitTest* test, const juce::String& subCategory) override
        {
            if (verbose)
                std::cout << "  Starting: " << subCategory << std::endl;
        }
        
        void endTest() override
        {
            testsPassed++;
        }
        
        void addFail(const juce::String& failureMessage) override
        {
            testsFailed++;
            std::cout << "  ❌ FAIL: " << failureMessage << std::endl;
        }
    };

    void showHelp()
    {
        std::cout << "TremoloViola Plugin Test Suite" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: TremoloViola_Tests [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --smoke       Run smoke tests only" << std::endl;
        std::cout << "  --unit        Run unit tests only" << std::endl;
        std::cout << "  --all         Run all tests (default)" << std::endl;
        std::cout << "  --verbose, -v Verbose output" << std::endl;
        std::cout << "  --help, -h    Show this help" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  TremoloViola_Tests                 # Run all tests" << std::endl;
        std::cout << "  TremoloViola_Tests --smoke         # Run smoke tests only" << std::endl;
        std::cout << "  TremoloViola_Tests --unit --verbose # Run unit tests with verbose output" << std::endl;
    }

    // Test instances
    SmokeTests smokeTests;
    TremoloLFOTests lfoTests;
    PresetManagerTests presetTests;
    PluginTests pluginTests;
};

//==============================================================================
START_JUCE_APPLICATION(TestApplication)