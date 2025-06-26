/*
  ==============================================================================

    SmokeTests.h
    Created: Smoke Tests for TremoloViola Plugin
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

/**
 * Smoke tests verify basic functionality and that the plugin can be instantiated
 * and perform basic operations without crashing. These are high-level integration tests.
 */
class SmokeTests : public juce::UnitTest
{
public:
    SmokeTests() : juce::UnitTest("Smoke Tests", "Integration") {}

    void runTest() override;

private:
    void testPluginInstantiation();
    void testBasicAudioProcessing();
    void testParameterManipulation();
    void testPresetLoading();
    void testStateRestoration();
};