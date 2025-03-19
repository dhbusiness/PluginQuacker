/*
  ==============================================================================

    FontManager.cpp
    Created: 19 Mar 2025 2:34:44pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "FontManager.h"

FontManager& FontManager::getInstance()
{
    static FontManager instance;
    return instance;
}

FontManager::FontManager()
{
    // Start with null typefaces
    regularTypeface = nullptr;
    boldTypeface = nullptr;
    italicTypeface = nullptr;
    
    // Load fonts immediately
    loadFonts();
}

FontManager::~FontManager()
{
    // Typeface smart pointers will handle cleanup automatically
}


bool FontManager::loadFonts()
{
    bool success = false;
    
    try {
        // Try to load custom fonts from binary resources
        auto regularFont = juce::Typeface::createSystemTypefaceFor(
            BinaryData::MontserratRegular_ttf,
            BinaryData::MontserratRegular_ttfSize);
        
        auto boldFont = juce::Typeface::createSystemTypefaceFor(
            BinaryData::MontserratBold_ttf,
            BinaryData::MontserratBold_ttfSize);
        
        // If loading was successful, store the typefaces
        if (regularFont != nullptr && boldFont != nullptr) {
            regularTypeface = regularFont;
            boldTypeface = boldFont;
            italicTypeface = regularFont;  // Use regular as fallback
            
            success = true;
            DBG("Successfully loaded Montserrat fonts");
        }
    }
    catch (const std::exception& e) {
        DBG("Exception loading fonts: " + juce::String(e.what()));
    }
    
    // If custom fonts couldn't be loaded, set up basic fallback fonts
    if (!success) {
        DBG("Using fallback fonts");
        
        // Create basic fonts as fallbacks
        juce::Font basicFont;
        juce::Font boldFont = basicFont.boldened();
        juce::Font italicFont = basicFont.italicised();
        
        regularTypeface = basicFont.getTypefacePtr();
        boldTypeface = boldFont.getTypefacePtr();
        italicTypeface = italicFont.getTypefacePtr();
    }
    
    return success;
}

juce::Font FontManager::getRegularFont(float size)
{
    juce::Font font(regularTypeface);
    font.setHeight(size);
    font.setExtraKerningFactor(0.01f);  // Slight adjustment for readability
    font.setHorizontalScale(1.0f);      // Ensure no horizontal stretching
    return font;
}

juce::Font FontManager::getBoldFont(float size)
{
    juce::Font font(boldTypeface);
    font.setHeight(size);
    font.setExtraKerningFactor(0.01f);  // Slight adjustment for readability
    font.setHorizontalScale(1.0f);      // Ensure no horizontal stretching
    return font;
}

juce::Font FontManager::getItalicFont(float size)
{
    juce::Font font(italicTypeface);
    font.setHeight(size);
    font.setExtraKerningFactor(0.01f);  // Slight adjustment for readability
    font.setHorizontalScale(1.0f);      // Ensure no horizontal stretching
    return font;
}

juce::Font FontManager::getHeadingFont(float size)
{
    juce::Font font(boldTypeface);
    font.setHeight(size);
    font.setExtraKerningFactor(0.01f);  // Slight adjustment for readability
    font.setHorizontalScale(1.0f);      // Ensure no horizontal stretching
    return font;
}

juce::Font FontManager::getLabelFont(float size)
{
    juce::Font font(regularTypeface);
    font.setHeight(size);
    font.setExtraKerningFactor(0.01f);  // Slight adjustment for readability
    font.setHorizontalScale(1.0f);      // Ensure no horizontal stretching
    return font;
}
