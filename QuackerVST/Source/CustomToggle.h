/*
  ==============================================================================

    CustomToggle.h
    Created: 1 Feb 2025 6:55:23pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class CustomToggle : public juce::LookAndFeel_V4
{
public:
    CustomToggle() {}

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        
        // Reserve space for label at top
        float labelHeight = 15.0f;
        auto switchBounds = bounds.withTrimmedTop(labelHeight);
        
        // Draw label
        g.setColour(juce::Colour(232, 193, 185));  // Light rose gold to match dial
        g.setFont(13.0f);
        g.drawText(button.getButtonText(), bounds.removeFromTop(labelHeight),
                   juce::Justification::centred, false);
        
        // Make switch smaller relative to its bounds
        float diameter = 30.0f;  // Reduced from 40.0f
        float innerDiameter = diameter * 0.85f;
        
        /*
        // Center the switch in the larger bounds
        juce::Rectangle<float> switchBounds(
            bounds.getCentreX() - diameter * 0.5f,
            bounds.getCentreY() - diameter * 0.5f,
            diameter,
            diameter
        );
         */
        
        // Outer ring shadow
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillEllipse(switchBounds.translated(1.0f, 1.0f));

        // Outer metal ring
        juce::ColourGradient ringGradient(
            juce::Colour(180, 180, 180), switchBounds.getX(), switchBounds.getY(),
            juce::Colour(140, 140, 140), switchBounds.getRight(), switchBounds.getBottom(),
            true
        );
        g.setGradientFill(ringGradient);
        g.fillEllipse(switchBounds);

        // Inner button area
        juce::Rectangle<float> innerBounds(
            bounds.getCentreX() - innerDiameter * 0.5f,
            bounds.getCentreY() - innerDiameter * 0.5f,
            innerDiameter,
            innerDiameter
        );

        // If pressed, move the button down slightly
        if (button.getToggleState()) {
            innerBounds.translate(0.0f, 2.0f);
        }

        // Button shadow
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillEllipse(innerBounds.translated(0.0f, 1.0f));

        // Main button surface
        juce::ColourGradient buttonGradient(
            button.getToggleState() ? juce::Colour(100, 100, 100) : juce::Colour(160, 160, 160),
            innerBounds.getX(), innerBounds.getY(),
            button.getToggleState() ? juce::Colour(80, 80, 80) : juce::Colour(120, 120, 120),
            innerBounds.getRight(), innerBounds.getBottom(),
            true
        );
        g.setGradientFill(buttonGradient);
        g.fillEllipse(innerBounds);

        // Add highlight to create depth
        if (!button.getToggleState()) {
            g.setColour(juce::Colours::white.withAlpha(0.4f));
            g.fillEllipse(innerBounds.reduced(innerDiameter * 0.2f).translated(0.0f, -innerDiameter * 0.1f));
        }

        // Add subtle circular grooves
        g.setColour(juce::Colours::black.withAlpha(0.1f));
        float grooveSpacing = innerDiameter * 0.15f;
        for (float i = 1; i <= 2; ++i) {
            g.drawEllipse(innerBounds.reduced(grooveSpacing * i), 0.5f);
        }

        // Draw LED
        float ledDiameter = 6.0f;
        juce::Rectangle<float> ledBounds(
            switchBounds.getX() + diameter/2 - ledDiameter/2,
            switchBounds.getY() - ledDiameter - 5.0f,
            ledDiameter,
            ledDiameter
        );

        // LED glow
        if (button.getToggleState()) {
            g.setColour(juce::Colour(19, 224, 139).withAlpha(0.3f));  // Using your teal color
            g.fillEllipse(ledBounds.expanded(2.0f));
        }

        // LED body
        g.setColour(button.getToggleState() ? juce::Colour(19, 224, 139) : juce::Colour(50, 50, 50));
        g.fillEllipse(ledBounds);
        
        // LED highlight
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.fillEllipse(ledBounds.reduced(ledDiameter * 0.4f));
    }
};
