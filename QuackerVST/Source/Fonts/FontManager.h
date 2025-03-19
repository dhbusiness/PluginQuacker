/*
  ==============================================================================

    FontManager.h
    Created: 19 Mar 2025 2:32:40pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "BinaryData.h" // Auto-generated header from Projucer

class FontManager
{
public:
    static FontManager& getInstance();
    
    juce::Font getRegularFont(float size = 16.0f);
    juce::Font getBoldFont(float size = 16.0f);
    juce::Font getItalicFont(float size = 16.0f);
    juce::Font getHeadingFont(float size = 18.0f);
    juce::Font getLabelFont(float size = 14.0f);
    
    juce::Typeface::Ptr getRegularTypeface() { return regularTypeface; }
    juce::Typeface::Ptr getBoldTypeface() { return boldTypeface; }
    
    // Initialize fonts - call this at startup
    bool loadFonts();

private:
    FontManager(); // Private constructor for singleton
    ~FontManager();
    
    // Font typefaces
    juce::Typeface::Ptr regularTypeface;
    juce::Typeface::Ptr boldTypeface;
    juce::Typeface::Ptr italicTypeface;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FontManager)
};
