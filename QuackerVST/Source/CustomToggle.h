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
        const float cornerSize = bounds.getHeight() * 0.5f; // Fully rounded corners
        
        // Colors
        juce::Colour buttonOffColor = juce::Colours::white.withAlpha(0.1f);
        juce::Colour buttonOnColor = juce::Colours::black.withAlpha(0.2f);  // Darker interior when on
        juce::Colour glowColor = juce::Colour(19, 224, 139);    // Teal
        juce::Colour textColor = juce::Colour(232, 193, 185);   // Light rose gold
        
        // Draw base background
        g.setColour(button.getToggleState() ? buttonOnColor : buttonOffColor);
        g.fillRoundedRectangle(bounds, cornerSize);

        if (button.getToggleState())
        {
            // Single sharp glowing border
            g.setColour(glowColor.withAlpha(0.8f));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
            
            // Very subtle outer glow
            g.setColour(glowColor.withAlpha(0.2f));
            g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 0.5f);
        }
        else
        {
            // Normal border
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }
        
        // Draw text
        g.setColour(button.getToggleState() ? glowColor.withAlpha(0.8f) : textColor);
        g.setFont(16.0f);
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
    }
};
