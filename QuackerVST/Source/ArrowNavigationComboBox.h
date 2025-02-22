/*
  ==============================================================================

    ArrowNavigationComboBox.h
    Created: 3 Feb 2025 11:44:20am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "CustomComboBox.h"

class ArrowNavigationComboBox : public juce::Component
{
public:
    class CustomArrowButton : public juce::Button
    {
    public:
        CustomArrowButton(bool isLeftArrow)
            : juce::Button(isLeftArrow ? "leftArrow" : "rightArrow"),
              pointingLeft(isLeftArrow)
        {
            setColour(buttonNormalColour, juce::Colour(232, 193, 185).withAlpha(0.6f));    // Rose gold
            setColour(buttonHighlightColour, juce::Colour(19, 224, 139));                  // Teal
        }

    protected:
        void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(6);  // Increased reduction for smaller size
            
            // Create triangle path
            juce::Path arrow;
            if (pointingLeft)
            {
                arrow.addTriangle(bounds.getRight(), bounds.getY(),
                                bounds.getX(), bounds.getCentreY(),
                                bounds.getRight(), bounds.getBottom());
            }
            else
            {
                arrow.addTriangle(bounds.getX(), bounds.getY(),
                                bounds.getRight(), bounds.getCentreY(),
                                bounds.getX(), bounds.getBottom());
            }

            // Draw filled triangle with glow effect when highlighted
            if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
            {
                // Glow effect
                g.setColour(findColour(buttonHighlightColour).withAlpha(0.2f));
                g.strokePath(arrow, juce::PathStrokeType(2.0f));
                
                // Main arrow
                g.setColour(findColour(buttonHighlightColour));
            }
            else
            {
                g.setColour(findColour(buttonNormalColour));
            }
            
            // Fill the triangle
            g.fillPath(arrow);
        }

    private:
        bool pointingLeft;
        
        enum ColourIds
        {
            buttonNormalColour = 0x1001,
            buttonHighlightColour = 0x1002
        };
    };
    ArrowNavigationComboBox()
    {
        leftArrow.reset(new CustomArrowButton(true));
        rightArrow.reset(new CustomArrowButton(false));
        
        addAndMakeVisible(comboBox);
        addAndMakeVisible(leftArrow.get());
        addAndMakeVisible(rightArrow.get());

        leftArrow->onClick = [this]() {
            int currentIndex = comboBox.getSelectedItemIndex();
            // Find previous valid (non-utility) item
            while (currentIndex > 0) {
                currentIndex--;
                if (!comboBox.getItemText(currentIndex).contains("...") &&
                    !comboBox.getItemText(currentIndex).contains("Open Preset") &&
                    !comboBox.getItemText(currentIndex).startsWith("-")) {
                    comboBox.setSelectedItemIndex(currentIndex);
                    break;
                }
            }
        };

        rightArrow->onClick = [this]() {
            int currentIndex = comboBox.getSelectedItemIndex();
            int numItems = comboBox.getNumItems();
            // Find next valid (non-utility) item
            while (currentIndex < numItems - 1) {
                currentIndex++;
                if (!comboBox.getItemText(currentIndex).contains("...") &&
                    !comboBox.getItemText(currentIndex).contains("Open Preset") &&
                    !comboBox.getItemText(currentIndex).startsWith("-")) {
                    comboBox.setSelectedItemIndex(currentIndex);
                    break;
                }
            }
        };
    }

    juce::ComboBox& getComboBox() { return comboBox; }

    void resized() override
    {
        auto bounds = getLocalBounds();
        const int arrowWidth = 25;  // Slightly wider for better visibility

        leftArrow->setBounds(bounds.removeFromLeft(arrowWidth));
        rightArrow->setBounds(bounds.removeFromRight(arrowWidth));
        comboBox.setBounds(bounds);
    }

private:
    juce::ComboBox comboBox;
    std::unique_ptr<CustomArrowButton> leftArrow;
    std::unique_ptr<CustomArrowButton> rightArrow;
};
