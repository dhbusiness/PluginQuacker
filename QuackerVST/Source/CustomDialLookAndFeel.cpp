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

    // Atmospheric shadow effect - more uniform and subtle
    for (int i = 0; i < 8; i++) {
        float shadowSize = diameter + (i * 2.0f);
        float alpha = 0.02f;
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.fillEllipse(centerX - (shadowSize * 0.5f),
                     centerY - (shadowSize * 0.5f),
                     shadowSize, shadowSize);
    }

    // Main metallic body gradient (rose gold)
    juce::ColourGradient mainGradient(
        juce::Colour(232, 193, 185), // Light rose gold
        centerX - radius, centerY - radius,
        juce::Colour(171, 136, 132), // Dark rose gold
        centerX + radius * 0.7f, centerY + radius * 0.7f,
        true);
    
    // Refined metallic bands
    mainGradient.addColour(0.2, juce::Colour(225, 185, 177));
    mainGradient.addColour(0.4, juce::Colour(200, 160, 155));
    mainGradient.addColour(0.6, juce::Colour(190, 150, 145));
    mainGradient.addColour(0.8, juce::Colour(180, 140, 135));

    // Main dial body
    g.setGradientFill(mainGradient);
    g.fillEllipse(centerX - radius, centerY - radius, diameter, diameter);

    // Draw dots with rich teal color
    const int numDots = 16;
    const float dotSize = 2.0f;
    const float startAngle = 0.75f * juce::MathConstants<float>::pi;
    const float arcSize = 1.5f * juce::MathConstants<float>::pi;
    const float dotSpacing = arcSize / (numDots - 1);
    
    // Pure teal base with luminescent glow
    juce::Colour tealColor = juce::Colour::fromString("#19E08B");     // Pure, rich teal
    juce::Colour tealGlow = tealColor.brighter(0.3f);    // Brighter only for glow
    
    for (int i = 0; i < numDots; ++i)
    {
        float dotAngle = startAngle + i * dotSpacing;
        float dotRadius = radius * 0.85f;
        float dotX = centerX + dotRadius * std::cos(dotAngle);
        float dotY = centerY + dotRadius * std::sin(dotAngle);
        
        float dotPosition = i / float(numDots - 1);
        
        if (dotPosition <= sliderPos)
        {
            // Enhanced luminescent glow effect
            // Outer glow - larger and softer
            g.setColour(tealGlow.withAlpha(0.1f));
            g.fillEllipse(dotX - dotSize * 2.2f, dotY - dotSize * 2.2f,
                         dotSize * 4.4f, dotSize * 4.4f);
            
            // Middle glow - more intense
            g.setColour(tealGlow.withAlpha(0.2f));
            g.fillEllipse(dotX - dotSize * 1.8f, dotY - dotSize * 1.8f,
                         dotSize * 3.6f, dotSize * 3.6f);
                         
            // Inner glow - brightest
            g.setColour(tealGlow.withAlpha(0.3f));
            g.fillEllipse(dotX - dotSize * 1.4f, dotY - dotSize * 1.4f,
                         dotSize * 2.8f, dotSize * 2.8f);
                         
            // Main dot
            g.setColour(tealColor);
            g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f,
                         dotSize, dotSize);
                         
            // Bright center highlight
            g.setColour(juce::Colours::white.withAlpha(0.9f));
            g.fillEllipse(dotX - dotSize * 0.2f, dotY - dotSize * 0.2f,
                         dotSize * 0.4f, dotSize * 0.4f);
        }
        else
        {
            // Inactive dots
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.fillEllipse(dotX - dotSize * 0.5f, dotY - dotSize * 0.5f,
                         dotSize, dotSize);
            
            // Subtle highlight for inactive dots
            g.setColour(juce::Colours::white.withAlpha(0.3f));
            g.fillEllipse(dotX - dotSize * 0.2f, dotY - dotSize * 0.2f,
                         dotSize * 0.4f, dotSize * 0.4f);
        }
    }
    // Enhanced pointer
    float sliderAngle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));
    
    // Subtle pointer shadow
    juce::Path pointerShadow;
    pointerShadow.startNewSubPath(centerX, centerY - radius * 0.2f);
    pointerShadow.lineTo(centerX, centerY - radius * 0.7f);
    
    g.setColour(juce::Colours::black.withAlpha(0.1f));
    g.strokePath(pointerShadow, juce::PathStrokeType(3.5f),
                juce::AffineTransform::rotation(sliderAngle, centerX, centerY)
                .translated(0.5f, 0.5f));

    // Main pointer with refined gradient
    juce::Path pointerPath;
    pointerPath.startNewSubPath(centerX, centerY - radius * 0.2f);
    pointerPath.lineTo(centerX, centerY - radius * 0.7f);
    
    g.setColour(juce::Colour(150, 120, 115));
    g.strokePath(pointerPath, juce::PathStrokeType(2.5f),
                juce::AffineTransform::rotation(sliderAngle, centerX, centerY));
}
