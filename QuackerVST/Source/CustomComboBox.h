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
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha(0.3f));
        setColour(juce::ComboBox::textColourId, juce::Colour(232, 193, 185));
        // Set arrow color to transparent to hide the default arrow
        setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);
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
    }

    // Override text positioning to be perfectly centered
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        // Remove any margin that might affect centering
        label.setBounds(1, 1, box.getWidth() - 2, box.getHeight() - 2);
        label.setJustificationType(juce::Justification::centred);
        
        // Set font
        label.setFont(16.0f);
    }

    // Style for popup menu
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
