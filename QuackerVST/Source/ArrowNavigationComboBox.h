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
    ArrowNavigationComboBox()
    {
        addAndMakeVisible(comboBox);
        addAndMakeVisible(leftButton);
        addAndMakeVisible(rightButton);

        leftButton.setButtonText("<");
        rightButton.setButtonText(">");

        leftButton.onClick = [this]() {
            int currentIndex = comboBox.getSelectedItemIndex();
            if (currentIndex > 0)
                comboBox.setSelectedItemIndex(currentIndex - 1);
        };

        rightButton.onClick = [this]() {
            int currentIndex = comboBox.getSelectedItemIndex();
            if (currentIndex < comboBox.getNumItems() - 1)
                comboBox.setSelectedItemIndex(currentIndex + 1);
        };
    }

    juce::ComboBox& getComboBox() { return comboBox; }

    void resized() override
    {
        auto bounds = getLocalBounds();
        const int buttonWidth = 20;

        leftButton.setBounds(bounds.removeFromLeft(buttonWidth));
        rightButton.setBounds(bounds.removeFromRight(buttonWidth));
        comboBox.setBounds(bounds);
    }

private:
    juce::ComboBox comboBox;
    juce::TextButton leftButton, rightButton;
};
