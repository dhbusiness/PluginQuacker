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

    // Shadow layers
    for (int i = 0; i < 8; i++) {
        float shadowSize = diameter + (i * 2.0f);
        float alpha = 0.03f;
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.fillEllipse(centerX - (shadowSize * 0.5f),
                     centerY - (shadowSize * 0.5f) + (i * 0.5f),
                     shadowSize, shadowSize);
    }

    // Main dial body
    g.setColour(juce::Colour(150, 140, 135));
    g.fillEllipse(centerX - radius, centerY - radius + 2, diameter, diameter);

    // Main body gradient
    juce::ColourGradient mainGradient(
        juce::Colour(232, 193, 185).brighter(0.1f),
        centerX - radius, centerY - radius,
        juce::Colour(171, 136, 132),
        centerX, centerY + radius,
        true);
    
    mainGradient.addColour(0.3, juce::Colour(225, 185, 177));
    mainGradient.addColour(0.5, juce::Colour(215, 175, 167));
    mainGradient.addColour(0.7, juce::Colour(200, 160, 155));
    mainGradient.addColour(0.9, juce::Colour(190, 150, 145));

    g.setGradientFill(mainGradient);
    g.fillEllipse(centerX - radius, centerY - radius, diameter, diameter);

    // Add central depression (restored)
    float innerDiameter = diameter * 0.85f;
    float innerRadius = innerDiameter * 0.5f;
    
    juce::ColourGradient innerGradient(
        juce::Colour(190, 150, 145),
        centerX - innerRadius, centerY - innerRadius,
        juce::Colour(210, 170, 165),
        centerX + innerRadius, centerY + innerRadius,
        true);
    
    g.setGradientFill(innerGradient);
    g.fillEllipse(centerX - innerRadius, centerY - innerRadius, innerDiameter, innerDiameter);

    // Draw dots further from the dial
    const int numDots = 28;
    const float dotSize = 2.0f;
    const float startAngle = 0.75f * juce::MathConstants<float>::pi;
    const float arcSize = 1.5f * juce::MathConstants<float>::pi;
    const float dotSpacing = arcSize / (numDots - 1);
    const float dotRadius = radius * 1.3f;  // Increased distance from dial
    
    juce::Colour tealColor = juce::Colour::fromString("#19E08B");
    juce::Colour tealGlow = tealColor.brighter(0.3f);
    
    for (int i = 0; i < numDots; ++i)
    {
        float dotAngle = startAngle + i * dotSpacing;
        float dotX = centerX + dotRadius * std::cos(dotAngle);
        float dotY = centerY + dotRadius * std::sin(dotAngle);
        
        float dotPosition = i / float(numDots - 1);
        
        if (dotPosition <= sliderPos)
        {
            // Enhanced glow effect
            g.setColour(tealGlow.withAlpha(0.1f));
            g.fillEllipse(dotX - dotSize * 2.2f, dotY - dotSize * 2.2f,
                         dotSize * 4.4f, dotSize * 4.4f);
            
            g.setColour(tealGlow.withAlpha(0.2f));
            g.fillEllipse(dotX - dotSize * 1.8f, dotY - dotSize * 1.8f,
                         dotSize * 3.6f, dotSize * 3.6f);
                         
            g.setColour(tealGlow.withAlpha(0.3f));
            g.fillEllipse(dotX - dotSize * 1.4f, dotY - dotSize * 1.4f,
                         dotSize * 2.8f, dotSize * 2.8f);
                         
            g.setColour(tealColor);
        }
        else
        {
            g.setColour(juce::Colours::white.withAlpha(0.5f));
        }
        
        g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f,
                      dotSize, dotSize);
                      
        // Add bright center to lit dots
        if (dotPosition <= sliderPos)
        {
            g.setColour(juce::Colours::white.withAlpha(0.9f));
            g.fillEllipse(dotX - dotSize * 0.2f, dotY - dotSize * 0.2f,
                         dotSize * 0.4f, dotSize * 0.4f);
        }
    }

    // Cutout pointer
    float sliderAngle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));
    
    juce::Path pointerPath;
    float pointerWidth = radius * 0.12f;  // Slightly thinner
    float pointerLength = radius * 0.4f;
    
    pointerPath.addRectangle(-pointerWidth * 0.5f, -radius * 0.85f, pointerWidth, pointerLength);
    
    // Darker shade of the dial color for the cutout base
    g.setColour(juce::Colour(171, 136, 132).darker(0.7f));  // Using dial's color but darker
    g.fillPath(pointerPath, juce::AffineTransform::rotation(sliderAngle)
               .translated(centerX, centerY));
    
    // Add subtle shadow inside the cutout
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.strokePath(pointerPath, juce::PathStrokeType(1.0f),
                juce::AffineTransform::rotation(sliderAngle)
                .translated(centerX + 0.5f, centerY + 0.5f));
    
    // Add very subtle highlight on one edge
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.strokePath(pointerPath, juce::PathStrokeType(0.5f),
                juce::AffineTransform::rotation(sliderAngle)
                .translated(centerX - 0.5f, centerY - 0.5f));
}
