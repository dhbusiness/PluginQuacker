/*
  ==============================================================================

    CustomDialLookAndFeel.cpp
    Created: 1 Feb 2025 1:04:17pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "CustomDialLookAndFeel.h"

CustomDial::CustomDial()
{

}

void CustomDial::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    float diameter = fmin(width, height) * 0.7;
    float radius = diameter * 0.5;
    float centerX = x + width * 0.5;
    float centerY = y + height * 0.5;
    float rx = centerX - radius;
    float ry = centerY - radius;
    float sliderAngle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));

    // Draw dots
    const int numDots = 32;
    const float dotSize = 2.0f;
    const float startAngle = 0.75f * juce::MathConstants<float>::pi;
    const float arcSize = 1.5f * juce::MathConstants<float>::pi; // 270 degrees
    const float dotSpacing = arcSize / (numDots - 1);
    
    for (int i = 0; i < numDots; ++i)
    {
        float dotAngle = startAngle + i * dotSpacing;
        float dotX = centerX + radius * std::cos(dotAngle);
        float dotY = centerY + radius * std::sin(dotAngle);
        
        // Calculate normalized position for this dot (0 to 1)
        float dotPosition = i / float(numDots - 1);
        
        // Light up dots based on slider position
        if (dotPosition <= sliderPos)
        {
            g.setColour(juce::Colours::cyan);
        }
        else
        {
            g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        }
        
        g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f,
                      dotSize, dotSize);
    }
    
    // Draw the pointer
    const float pointerLength = radius * 0.9f;
    const float pointerThickness = 3.0f;
    
    juce::Path pointerPath;
    pointerPath.startNewSubPath(centerX, centerY - radius * 0.2f);
    pointerPath.lineTo(centerX, centerY - pointerLength);
    
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(pointerPath, juce::PathStrokeType(pointerThickness),
                juce::AffineTransform::rotation(sliderAngle, centerX, centerY));
}
