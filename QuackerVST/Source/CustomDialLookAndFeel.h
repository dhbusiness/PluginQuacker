/*
  ==============================================================================

    CustomDialLookAndFeel.h
    Created: 1 Feb 2025 12:13:48pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class CustomDial : public juce::LookAndFeel_V4
{
public:
    CustomDial();


    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
};
