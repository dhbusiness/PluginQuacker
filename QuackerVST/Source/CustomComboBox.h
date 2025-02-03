/*
  ==============================================================================

    CustomComboBox.h
    Created: 3 Feb 2025 11:32:21am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class CustomComboBox : public juce::LookAndFeel_V4
{
public:
    CustomComboBox()
    {
        // Set custom colors
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha(0.3f));
        setColour(juce::ComboBox::textColourId, juce::Colour(232, 193, 185));  // Rose gold
        setColour(juce::ComboBox::arrowColourId, juce::Colour(19, 224, 139));  // Teal
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH,
                     juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float>(0, 0, width, height);
        const float cornerSize = 6.0f;
        
        // Draw main background
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(bounds, cornerSize);

        // Draw border
        g.setColour(juce::Colour(171, 136, 132).withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);

        // Draw right arrow (only one arrow now)
        auto rightArrowBounds = bounds.removeFromRight(height).reduced(8);
        juce::Path rightArrow;
        rightArrow.startNewSubPath(rightArrowBounds.getCentreX() - 4, rightArrowBounds.getCentreY() - 4);
        rightArrow.lineTo(rightArrowBounds.getCentreX() + 4, rightArrowBounds.getCentreY());
        rightArrow.lineTo(rightArrowBounds.getCentreX() - 4, rightArrowBounds.getCentreY() + 4);
        g.setColour(juce::Colour(19, 224, 139).withAlpha(0.8f));
        g.strokePath(rightArrow, juce::PathStrokeType(1.0f));
    }

    // Override text positioning
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(0, 0, box.getWidth() - box.getHeight(), box.getHeight());
        label.setJustificationType(juce::Justification::centred);
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        g.fillAll(juce::Colours::black.withAlpha(0.95f));
        g.setColour(juce::Colour(171, 136, 132).withAlpha(0.5f));
        g.drawRect(0, 0, width, height, 1);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted,
                          bool isTicked, bool hasSubMenu, const juce::String& text,
                          const juce::String& shortcutKeyText, const juce::Drawable* icon,
                          const juce::Colour* textColour) override
    {
        if (isHighlighted)
        {
            g.setColour(juce::Colour(19, 224, 139).withAlpha(0.2f));
            g.fillRect(area);
        }

        g.setColour(isHighlighted ? juce::Colour(19, 224, 139) : juce::Colour(232, 193, 185));
        g.setFont(16.0f);
        g.drawText(text, area.reduced(10, 0), juce::Justification::centred);
    }
};
