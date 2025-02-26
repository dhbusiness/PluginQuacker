/*
  ==============================================================================

    TransparentButtonLookAndFeel.h
    Created: 26 Feb 2025 10:15:42am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class TransparentButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TransparentButtonLookAndFeel()
    {
        // Keep the button itself transparent
        setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    }
    
    
    void drawButtonBackground(juce::Graphics&, juce::Button&,
                             const juce::Colour&,
                             bool, bool) override
    {
        // Deliberately empty to prevent any background drawing
    }
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                       bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds();
        
        // Main theme colors matching your popup menu
        juce::Colour roseGold = juce::Colour(232, 193, 185);
        juce::Colour tealHighlight = juce::Colour(19, 224, 139);
        
        // Font styling to match the menu
        g.setFont(juce::Font(16.0f).withStyle(juce::Font::plain));
        
        // Text color based on state
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            // Use teal text when highlighted, like in the menu
            g.setColour(tealHighlight);
        }
        else
        {
            // Use rose gold for normal state
            g.setColour(roseGold);
        }
        
        // Draw the text
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred, true);
    }
};
