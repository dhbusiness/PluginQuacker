/*
  ==============================================================================

    CustomMenuLookAndFeel.h
    Created: 26 Feb 2025 9:32:18am
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PerlinNoise.h"

/**
 * A custom look and feel for popup menus that matches the style of the Quacker plugin UI.
 */
class CustomMenuLookAndFeel : public juce::LookAndFeel_V4
{
    public:
    CustomMenuLookAndFeel()
    {
        // Create the background image once
        backgroundImage = createBackgroundImage(400, 400);
        
        // Set colors for various components
        setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black.withAlpha(0.92f));
        setColour(juce::PopupMenu::textColourId, juce::Colour(232, 193, 185)); // Rose gold
        setColour(juce::PopupMenu::headerTextColourId, juce::Colour(232, 193, 185)); // Rose gold
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(19, 224, 139).withAlpha(0.2f)); // Teal highlight
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(19, 224, 139)); // Teal text for highlighted items
    }
    
    private:
    juce::Image backgroundImage;
    
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        // Draw a dark, semi-transparent background
        g.setColour(findColour(juce::PopupMenu::backgroundColourId));
        g.fillAll();
        
        // Draw our background image with reduced opacity
        if (!backgroundImage.isNull())
        {
            g.setOpacity(0.6f);
            g.drawImage(backgroundImage,
                        0, 0, width, height,
                        0, 0, backgroundImage.getWidth(), backgroundImage.getHeight(),
                        false);
        }
        
        // Add a subtle border
        g.setColour(juce::Colour(171, 136, 132).withAlpha(0.5f)); // Darker rose gold
        g.drawRect(0, 0, width, height, 1);
    }
    
    // Create a background image with matching style to the plugin
    juce::Image createBackgroundImage(int width, int height)
    {
        juce::Image img(juce::Image::ARGB, width, height, true);
        juce::Graphics g(img);
        
        auto bounds = juce::Rectangle<float>(0, 0, width, height);
        
        // Enhanced color palette with metallic tones
        juce::Colour darkPlum(61, 21, 46);
        juce::Colour midPlum(72, 28, 55);
        juce::Colour lightPlum(89, 34, 68);
        juce::Colour peachPink = juce::Colour(255, 201, 190).withMultipliedBrightness(0.6f);
        juce::Colour roseGold = juce::Colour(232, 193, 185).withMultipliedBrightness(0.6f);
        juce::Colour warmPlum = juce::Colour(198, 109, 139).withMultipliedBrightness(0.7f);
        
        // Metallic sheen colors
        juce::Colour metalHighlight = juce::Colour(255, 255, 255).withAlpha(0.03f);
        juce::Colour metalShadow = juce::Colours::black.withAlpha(0.05f);
        
        // Base gradient
        juce::ColourGradient baseGradient(
                                          darkPlum.brighter(0.05f),
                                          bounds.getCentreX(), bounds.getCentreY(),
                                          midPlum.darker(0.05f),
                                          bounds.getRight(), bounds.getBottom(),
                                          true);
        
        baseGradient.addColour(0.3, midPlum);
        baseGradient.addColour(0.7, lightPlum);
        
        g.setGradientFill(baseGradient);
        g.fillAll();
        
        // Add initial sheen
        juce::ColourGradient sheenGradient(
                                           metalHighlight,
                                           bounds.getCentreX(), bounds.getCentreY(),
                                           metalShadow,
                                           bounds.getRight(), bounds.getBottom(),
                                           true);
        
        g.setGradientFill(sheenGradient);
        g.fillAll();
        
        // Add Perlin noise patterns
        const float fixedSeed = 42.0f;
        
        struct LayerConfig {
            float scale;
            float alpha;
            float amplitude;
            juce::Colour color;
            float offset;
        };
        
        std::array<LayerConfig, 3> layers = {{
            { 0.003f, 0.08f, 8.0f, warmPlum, 0.0f },
            { 0.005f, 0.06f, 6.0f, roseGold, 50.0f },
            { 0.008f, 0.04f, 5.0f, peachPink, 100.0f }
        }};
        
        // Generate texture layers
        for (const auto& layer : layers)
        {
            juce::Path swirlyPath;
            
            for (float y = 0; y < bounds.getHeight(); y += 4.0f)
            {
                swirlyPath.startNewSubPath(0, y);
                
                for (float x = 0; x < bounds.getWidth(); x += 4.0f)
                {
                    float noise1 = PerlinNoise::noise(x * layer.scale, y * layer.scale, fixedSeed + layer.offset);
                    float noise2 = PerlinNoise::noise(x * layer.scale * 1.7f, y * layer.scale * 1.7f, fixedSeed + layer.offset + 10.0f);
                    
                    float combinedNoise = (noise1 + noise2 * 0.5f) * juce::MathConstants<float>::pi * 4;
                    
                    float offsetX = std::sin(combinedNoise) * layer.amplitude;
                    float offsetY = std::cos(combinedNoise) * layer.amplitude;
                    
                    swirlyPath.lineTo(x + offsetX, y + offsetY);
                }
            }
            
            g.setColour(layer.color.withAlpha(layer.alpha));
            g.strokePath(swirlyPath, juce::PathStrokeType(1.5f));
        }
        
        // Add metal flakes (subtle sparkle effect)
        juce::Random random;
        for (int i = 0; i < 3000; ++i)
        {
            float x = random.nextFloat() * width;
            float y = random.nextFloat() * height;
            float size = random.nextFloat() * 1.4f;
            float alpha = random.nextFloat() * 0.05f;
            
            if (random.nextBool()) {
                g.setColour(juce::Colours::white.withAlpha(alpha));
            } else {
                g.setColour(metalShadow.withAlpha(alpha * 0.8f));
            }
            
            g.fillEllipse(x, y, size, size);
        }
        
        return img;
    }
    
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                           bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override
    {
        if (isSeparator)
        {
            // Style separators like section headings with rose gold color
            g.setColour(findColour(juce::PopupMenu::headerTextColourId));
            g.setFont(juce::Font(20.0f).boldened());
            g.drawText(text, area.reduced(15, 0), juce::Justification::left, true);
            
            // Subtle underline
            g.setColour(findColour(juce::PopupMenu::headerTextColourId).withAlpha(0.3f));
            g.drawLine(area.getX() + 4, area.getBottom() - 2,
                       area.getRight() - 4, area.getBottom() - 2,
                       1.0f);
            return;
        }
        
        // Draw highlighted background
        if (isHighlighted && isActive)
        {
            // Fill with highlight color
            g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRect(area);
            
            // Add highlight border
            g.setColour(findColour(juce::PopupMenu::highlightedTextColourId).withAlpha(0.5f));
            g.drawRect(area.reduced(1), 1.0f);
        }
        
        // Text color based on state
        juce::Colour itemTextColour;
        
        if (textColour != nullptr)
        {
            itemTextColour = *textColour;
        }
        else if (!isActive)
        {
            itemTextColour = findColour(juce::PopupMenu::textColourId).withAlpha(0.4f);
        }
        else if (isHighlighted)
        {
            itemTextColour = findColour(juce::PopupMenu::highlightedTextColourId);
        }
        else
        {
            itemTextColour = findColour(juce::PopupMenu::textColourId);
        }
        
        g.setColour(itemTextColour);
        g.setFont(16.0f);
        
        // Draw the text with appropriate padding
        juce::Rectangle<int> textArea = area.reduced(hasSubMenu ? 30 : 20, 0);
        g.drawText(text, textArea, juce::Justification::left, true);
        
        // Draw tick if needed
        if (isTicked)
        {
            const float tickSize = 6.0f;
            const float tickX = area.getX() + 6.0f;
            const float tickY = area.getCentreY() - (tickSize / 2.0f);
            
            g.setColour(itemTextColour);
            g.fillEllipse(tickX, tickY, tickSize, tickSize);
        }
        
        // Draw arrow for submenus
        if (hasSubMenu)
        {
            const float arrowSize = 5.0f;
            const float arrowX = area.getRight() - 15.0f;
            const float arrowY = area.getCentreY();
            
            juce::Path path;
            path.addTriangle(arrowX, arrowY - arrowSize,
                             arrowX, arrowY + arrowSize,
                             arrowX + arrowSize, arrowY);
            
            g.fillPath(path);
        }
    }
};
