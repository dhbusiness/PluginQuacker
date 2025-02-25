/*
  ==============================================================================

    PresetComponent.h
    Created: 21 Feb 2025 7:40:21pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/


#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "CustomComboBox.h"
#include "CustomToggle.h"
#include "ArrowNavigationComboBox.h"
#include "PerlinNoise.h"

class PresetComponent : public juce::Component,
                        public juce::ComboBox::Listener,
                        public juce::Timer
{
public:
    PresetComponent(PresetManager& pm);
    ~PresetComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void updatePresetList();

    // Timer callback to update the displayed preset name
    void timerCallback() override;

private:
    PresetManager& presetManager;
    ArrowNavigationComboBox presetSelector;
    
    // Styling constants
    const float cornerRadius = 3.0f;  // Reduced corner radius for more minimal look
    const juce::Colour textColour = juce::Colour(232, 193, 185).withAlpha(0.8f);
    const juce::Colour backgroundColour = juce::Colours::black.withAlpha(0.2f);
    const juce::Colour borderColour = juce::Colours::white.withAlpha(0.1f);
    
    void showSavePresetDialog();
    void loadSelectedPreset();


    class PresetSelectorLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        PresetSelectorLookAndFeel()
        {
            // Initialize with larger dimensions for smoother noise
            backgroundImage = createBackgroundImage(300, 300);
        }
        
        // Create a more sophisticated background image that matches PluginEditor style
        juce::Image createBackgroundImage(int width, int height)
        {
            juce::Image img(juce::Image::ARGB, width, height, true);
            juce::Graphics g(img);
            
            auto bounds = juce::Rectangle<float>(0, 0, width, height);
            
            // Enhanced color palette with metallic tones (matching PluginEditor)
            juce::Colour darkPlum(61, 21, 46);
            juce::Colour midPlum(72, 28, 55);
            juce::Colour lightPlum(89, 34, 68);
            juce::Colour peachPink = juce::Colour(255, 201, 190).withMultipliedBrightness(0.6f);
            juce::Colour roseGold = juce::Colour(232, 193, 185).withMultipliedBrightness(0.6f);
            juce::Colour warmPlum = juce::Colour(198, 109, 139).withMultipliedBrightness(0.7f);
            
            // Metallic sheen colors
            juce::Colour metalHighlight = juce::Colour(255, 255, 255).withAlpha(0.03f);
            juce::Colour metalShadow = juce::Colours::black.withAlpha(0.05f);

            // Base gradient (just like in PluginEditor)
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

            // Add initial sheen just like in PluginEditor
            juce::ColourGradient sheenGradient(
                metalHighlight,
                bounds.getCentreX(), bounds.getCentreY(),
                metalShadow,
                bounds.getRight(), bounds.getBottom(),
                true);
            
            g.setGradientFill(sheenGradient);
            g.fillAll();

            // Add Perlin noise patterns with multiple layers
            const float fixedSeed = 42.0f;  // Same seed as PluginEditor for consistency
            
            struct LayerConfig {
                float scale;
                float alpha;
                float amplitude;
                juce::Colour color;
                float offset;
            };

            // Define layers with the same configuration as PluginEditor
            std::array<LayerConfig, 5> layers = {{
                { 0.003f, 0.08f, 8.0f, warmPlum, 0.0f },
                { 0.005f, 0.06f, 6.0f, roseGold, 50.0f },
                { 0.008f, 0.04f, 5.0f, peachPink, 100.0f },
                { 0.002f, 0.08f, 10.0f, lightPlum, 150.0f },
                { 0.004f, 0.03f, 6.0f, warmPlum, 200.0f }
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
                        float noise3 = PerlinNoise::noise(y * layer.scale * 0.5f, x * layer.scale * 0.5f, fixedSeed + layer.offset + 20.0f);
                        
                        float combinedNoise = (noise1 + noise2 * 0.5f + noise3 * 0.25f) * juce::MathConstants<float>::pi * 4;
                        
                        float offsetX = std::sin(combinedNoise) * layer.amplitude;
                        float offsetY = std::cos(combinedNoise) * layer.amplitude;
                        
                        swirlyPath.lineTo(x + offsetX, y + offsetY);
                    }
                }
                
                g.setColour(layer.color.withAlpha(layer.alpha));
                g.strokePath(swirlyPath, juce::PathStrokeType(1.5f));
            }

            // Add metal flakes (subtle sparkle effect) just like in PluginEditor
            juce::Random random;
            for (int i = 0; i < 5000; ++i)  // Reduced count for menu background
            {
                float x = random.nextFloat() * width;
                float y = random.nextFloat() * height;
                float size = random.nextFloat() * 1.4f;
                float alpha = random.nextFloat() * 0.05f;  // Reduced intensity for menu

                if (random.nextBool()) {
                    g.setColour(juce::Colours::white.withAlpha(alpha));
                } else {
                    g.setColour(metalShadow.withAlpha(alpha * 0.8f));
                }
                
                g.fillEllipse(x, y, size, size);
            }

            // Final overall sheen just like in PluginEditor
            juce::ColourGradient finalSheen(
                metalHighlight.withAlpha(0.01f),
                bounds.getCentreX(), bounds.getCentreY(),
                metalShadow.withAlpha(0.01f),
                bounds.getRight(), bounds.getBottom(),
                true);
            
            g.setGradientFill(finalSheen);
            g.fillAll();
            
            return img;
        }
        
        // Draw the popup menu background using our pre-generated background image
        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            // First fill with a slightly transparent black to ensure readability
            g.setColour(juce::Colours::black.withAlpha(0.85f));
            g.fillAll();
            
            // Draw our background image with reduced opacity
            if (!backgroundImage.isNull())
            {
                g.setOpacity(0.6f);  // Transparency level to blend with the black background
                g.drawImage(backgroundImage,
                             0, 0, width, height,
                             0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
            }
            
            // Add a subtle border
            g.setColour(juce::Colour(171, 136, 132).withAlpha(0.5f));  // Darker rose gold
            g.drawRect(0, 0, width, height, 1);
        }
        
        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                         int buttonX, int buttonY, int buttonW, int buttonH,
                         juce::ComboBox& box) override
        {
            // Draw nothing - this gives us a completely transparent background
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            // Position the text in the center of the entire combo box
            label.setBounds(0, 0, box.getWidth(), box.getHeight());
            label.setJustificationType(juce::Justification::centred);
        }
        
        void drawPopupMenuSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& sectionName) override
        {
            // Create a font with extra letter spacing
            juce::Font headerFont = juce::Font(20.0f).boldened();
            headerFont.setExtraKerningFactor(0.05f); // Add 10% extra space between letters
            g.setFont(headerFont);
            
            // Calculate positions for the dot symbol and text
            float dotSize = 4.0f; // Small dot size for subtlety
            float dotY = area.getCentreY() - dotSize/2;
            float textX = area.getX() + 15.0f; // Padding for text
            
            // Draw the text in rose gold
            g.setColour(juce::Colour(232, 193, 185));
            g.drawText(sectionName, area.reduced(15, 0), juce::Justification::left, true);
            
            // Draw a small dot at the start of the section name
            g.fillEllipse(area.getX() + 7.0f, dotY, dotSize, dotSize);
            
            // Draw underline
            g.setColour(juce::Colour(232, 193, 185).withAlpha(0.3f));
            g.drawLine(area.getX() + 4, area.getBottom() - 2,
                      area.getRight() - 4, area.getBottom() - 2,
                      1.0f);
        }
        
        // Draw each popup menu item with better styling that matches the PluginEditor aesthetic
        void drawPopupMenuItem(juce::Graphics& g,
                              const juce::Rectangle<int>& area,
                              bool isSeparator,
                              bool isActive,
                              bool isHighlighted,
                              bool isTicked,
                              bool hasSubMenu,
                              const juce::String& text,
                              const juce::String& shortcutKeyText,
                              const juce::Drawable* icon,
                              const juce::Colour* textColour) override
        {
            if (isSeparator)
            {
                // Style separators like section headings with rose gold color
                g.setColour(juce::Colour(232, 193, 185));  // Full rose gold color without transparency
                g.setFont(juce::Font(20.0f).boldened());   // Keep the large, bold font
                g.drawText(text, area.reduced(15, 0), juce::Justification::left, true);
                
                // Optional: keep a subtle underline if desired
                g.setColour(juce::Colour(232, 193, 185).withAlpha(0.3f));
                g.drawLine(area.getX() + 4, area.getBottom() - 2,
                            area.getRight() - 4, area.getBottom() - 2,
                            1.0f);
                return;
            }
            
            // Draw highlighted background with teal glow, matching the custom button style
            if (isHighlighted && isActive)
            {
                // Base highlight color (teal, matching buttons in PluginEditor)
                juce::Colour tealColor = juce::Colour::fromString("#19E08B");
                
                // Glow effect with gradient
                g.setColour(tealColor.withAlpha(0.2f));
                g.fillRect(area);
                
                // Highlight border
                g.setColour(tealColor.withAlpha(0.5f));
                g.drawRect(area.reduced(1), 1.0f);
            }
            
            // Text color based on state
            juce::Colour itemTextColour;
            
            if (!isActive)
                itemTextColour = juce::Colour(232, 193, 185).withAlpha(0.4f);
            else if (isHighlighted)
                itemTextColour = juce::Colour(19, 224, 139);  // Teal, like selected elements
            else
                itemTextColour = juce::Colour(232, 193, 185);  // Rose gold, like other UI elements
            
            g.setColour(itemTextColour);
            g.setFont(16.0f);
            
            // Draw the text with appropriate padding
            juce::Rectangle<int> textArea = area.reduced(20, 0);
            g.drawText(text, textArea, juce::Justification::left, true);
        }
        
    private:
        juce::Image backgroundImage;
    };

private:
    PresetSelectorLookAndFeel presetLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetComponent)
};
