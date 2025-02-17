/*
  ==============================================================================

    CustomDialLookAndFeel.h
    Created: 1 Feb 2025 12:13:48pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class CustomDial : public juce::LookAndFeel_V4, private juce::Timer
{
public:
    CustomDial();
    ~CustomDial() override;

    // Declare the methods that will be implemented in the cpp file
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, const float rotaryStartAngle,
                         const float rotaryEndAngle, juce::Slider& slider) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

private:
    void timerCallback() override;

    // Storage for opacity values
    std::map<const juce::Slider*, float> sliderTargetOpacities;
    std::map<const juce::Slider*, float> sliderCurrentOpacities;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomDial)
};
