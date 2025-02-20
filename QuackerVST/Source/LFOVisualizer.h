/*
  ==============================================================================

    LFOVisualizer.h
    Created: 30 Jan 2025 3:21:36pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PerlinNoise.h"

class LFOVisualizer : public juce::Component, public juce::Timer
{
public:
    LFOVisualizer() : currentPhase(0.0)
    {
        startTimerHz(144); // Increase refresh rate for smoother animation
    }

    ~LFOVisualizer() override
    {
        stopTimer();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto originalBounds = bounds; // Store original bounds for border

        // Enhanced background gradient
        juce::ColourGradient bgGradient(
            juce::Colours::black.brighter(0.1f), bounds.getX(), bounds.getY(),
            juce::Colours::black.darker(0.2f), bounds.getX(), bounds.getBottom(),
            false);
        g.setGradientFill(bgGradient);
        g.fillRect(bounds);

        // Subtle vignette effect
        juce::ColourGradient vignette(
            juce::Colours::transparentBlack,
            bounds.getCentreX(), bounds.getCentreY(),
            juce::Colours::black.withAlpha(0.3f),
            bounds.getX(), bounds.getY(),
            true);
        g.setGradientFill(vignette);
        g.fillRect(bounds);

        // Draw rate indicator
        g.setColour(juce::Colour(19, 224, 139).withAlpha(0.8f));
        g.setFont(12.0f);
        juce::String rateText;
        if (tempoSynced)
        {
            const char* divisions[] = { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" };
            rateText = juce::String(bpm, 1) + " BPM - " + juce::String(divisions[noteDivision]);
        }
        else
        {
            rateText = juce::String(rate, 2) + " Hz";
        }
        
        // Remove top area for rate display from working bounds
        auto textBounds = bounds.removeFromTop(20);
        g.drawText(rateText, textBounds, juce::Justification::centred);
        
        // Subtle moving scan lines - Made much more subtle
        g.setColour(juce::Colours::white.withAlpha(0.015f)); // Reduced opacity
        float scanLineSpacing = 4.0f;
        float scanLineOffset = crtPhase * bounds.getHeight() * 2.0f;
        for (float y = -scanLineSpacing; y <= bounds.getHeight() + scanLineSpacing; y += scanLineSpacing)
        {
            float actualY = std::fmod(y + scanLineOffset, bounds.getHeight());
            g.drawHorizontalLine(static_cast<int>(actualY), 0.0f, bounds.getWidth());
        }

        // Enhanced grid drawing
                g.setColour(juce::Colour(232, 193, 185).withAlpha(0.1f));
        
        // Calculate grid size based on available space
        int numVerticalDivisions = 8;
        float gridSizeY = bounds.getHeight() / numVerticalDivisions;
        float gridSizeX = gridSizeY; // Keep squares proportional
        
        // Vertical grid lines
        int numHorizontalDivisions = static_cast<int>(std::ceil(bounds.getWidth() / gridSizeX));
        for (int i = 0; i <= numHorizontalDivisions; ++i)
        {
            float x = i * gridSizeX;
            g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
        }
        
        // Horizontal grid lines
        for (int i = 0; i <= numVerticalDivisions; ++i)
        {
            float y = bounds.getY() + (i * gridSizeY);
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, bounds.getWidth());
        }

        // Center line with slightly more emphasis
        g.setColour(juce::Colours::darkgrey.withAlpha(0.4f));
        float midY = bounds.getCentreY();
        g.drawHorizontalLine(static_cast<int>(midY), 0.0f, bounds.getWidth());
        
        // Draw waveform with dynamic thickness
        // Create our teal-focused gradient colors
        juce::Colour mainTeal = juce::Colour(19, 224, 139);          // Base teal
        juce::Colour brightTeal = mainTeal.brighter(0.2f);           // Brighter for peaks

        
        // Store path points for both original and shaped waveforms
        std::vector<std::pair<float, float>> originalPoints;
        std::vector<std::pair<float, float>> shapedPoints;
        
        const float pointsPerPixel = 1.0f;
        const int numPoints = static_cast<int>(bounds.getWidth() * pointsPerPixel);

        std::vector<std::pair<float, float>> waveformPoints;

        // Calculate points for the waveform
        for (int i = 0; i < numPoints; ++i)
        {
            float x = (static_cast<float>(i) / static_cast<float>(numPoints - 1)) * bounds.getWidth();
            float phase = (static_cast<float>(i) / static_cast<float>(numPoints - 1) + currentPhase);
            phase += phaseOffset / 360.0f;

            while (phase >= 1.0f) phase -= 1.0f;
            while (phase < 0.0f) phase += 1.0f;

            // Get our base waveform value
            float waveformValue = calculateWaveformValue(phase, currentWaveform);
            
            // If waveshaping is enabled, apply it
            if (waveshapeEnabled) {
                float shapePhase = std::fmod(phase * waveshapeRate + currentPhase, 1.0f);
                float shapingValue = calculateWaveformValue(shapePhase, waveshapeWaveform);
                
                // Apply waveshaping modulation
                waveformValue = waveformValue + (shapingValue * waveshapeDepth);
                // Limit to -1 to 1 range
                waveformValue = juce::jlimit(-1.0f, 1.0f, waveformValue);
            }

            // Calculate y position
            float y = bounds.getCentreY() - (waveformValue * depth * bounds.getHeight() * 0.4f);
            waveformPoints.push_back({x, y});
        }

        drawDynamicWaveform(g, waveformPoints, juce::Colour(19, 224, 139));


        // Draw border LAST, using the ORIGINAL bounds
        g.setColour(juce::Colour(120, 80, 75));
        g.drawRect(originalBounds, 1.0f); // Use originalBounds instead of bounds
    }

    void drawDynamicWaveform(juce::Graphics& g,
                            const std::vector<std::pair<float, float>>& points,
                            juce::Colour baseColor)
    {
        auto bounds = getLocalBounds().toFloat();
        float centerY = bounds.getCentreY();
        
        // Our base teal color
        juce::Colour mainTeal = juce::Colour(19, 224, 139);
        juce::Colour brightTeal = mainTeal.brighter(0.2f);
        
        for (size_t i = 0; i < points.size() - 1; ++i) {
            float x1 = points[i].first;
            float y1 = points[i].second;
            float x2 = points[i + 1].first;
            float y2 = points[i + 1].second;
            
            // Calculate how far we are from center (normalized 0-1)
            float distanceFromCenter = std::abs(y1 - centerY);
            float normalizedDistance = juce::jlimit(0.0f, 1.0f,
                distanceFromCenter / (bounds.getHeight() * 0.4f));
            
            // Create gradient based on distance from center
            juce::Colour lineColor = normalizedDistance > 0.5f ?
                brightTeal : mainTeal;
            
            // Outer glow
            g.setColour(lineColor.withAlpha(0.15f));
            g.drawLine(x1, y1, x2, y2, 6.0f);
            
            // Middle glow
            g.setColour(lineColor.withAlpha(0.3f));
            g.drawLine(x1, y1, x2, y2, 3.5f);
            
            // Core line
            g.setColour(lineColor.withAlpha(0.95f));
            g.drawLine(x1, y1, x2, y2, 2.0f);
            
            // Bright center
            g.setColour(lineColor.brighter(0.2f).withAlpha(0.8f));
            g.drawLine(x1, y1, x2, y2, 0.5f);
        }
    };

    
    void resized() override {}


    void setActive(bool shouldBeActive, bool isWaitingForReset)
    {
        if (!shouldBeActive && !isWaitingForReset)
        {
            // Only reset phase if we're not waiting to complete a cycle
            currentPhase = 0.0;
        }
        active = shouldBeActive;
        waitingForReset = isWaitingForReset;
    }
    
    void setWaitingForReset(bool waiting)
    {
        waitingForReset = waiting;
    }
    
    void timerCallback() override
    {
        // Always update CRT animation phase
        crtPhase += 0.01f; // Adjust speed as needed
        while (crtPhase >= 1.0f)
            crtPhase -= 1.0f;

        // LFO phase update only when active
        if (active || waitingForReset)
        {
            if (tempoSynced)
            {
                const double quarterNoteRate = bpm / 60.0;
                const double multipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
                double frequencyHz = quarterNoteRate * multipliers[noteDivision] * 2.0;
                double phaseIncrement = frequencyHz / 100.0;
                currentPhase += phaseIncrement;
            }
            else
            {
                currentPhase += rate / 100.0;
            }

            // Only reset if explicitly waiting for reset and at reset point
            if (waitingForReset && (currentPhase >= 0.99 || currentPhase < 0.01))
            {
                waitingForReset = false;
                currentPhase = 0.0;
            }
            else
            {
                while (currentPhase >= 1.0)
                    currentPhase -= 1.0;
            }
        }
        
        repaint();
    }


    void setWaveform(int waveformType)
    {
        currentWaveform = waveformType;
    }

    void setDepth(float newDepth)
    {
        depth = newDepth;
    }

    void setPhaseOffset(float newPhaseOffset)
    {
        phaseOffset = newPhaseOffset;
    }

    void setRate(float newRate)
    {
        rate = newRate;
        tempoSynced = false;
    }

    void setTempoSync(bool synced, double newBpm, int division)
    {
        tempoSynced = synced;
        bpm = newBpm;
        noteDivision = division;
    }
    
    void setWaveshapeParameters(float depth, float rate, int waveform, bool enabled) {
        waveshapeDepth = depth;
        waveshapeRate = rate;
        waveshapeWaveform = waveform;
        waveshapeEnabled = enabled;
    }

private:
    
    float calculateWaveformValue(float phase, int waveform)
    {
        switch (waveform)
        {
            case 0: // Sine
                return std::sin(phase * juce::MathConstants<float>::twoPi);
                
            case 1: // Square
                return (phase < 0.5f) ? 1.0f : -1.0f;
                
            case 2: // Triangle
                return 2.0f * (phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f) - 1.0f;
                
            case 3: // Sawtooth
                return 2.0f * phase - 1.0f;
                
            case 4: // Ramp Down
                return 1.0f - (2.0f * phase);
                
            case 5: // Soft Square
                {
                    const float sharpness = 10.0f;
                    float centered = phase * 2.0f - 1.0f;
                    return 2.0f * (1.0f / (1.0f + std::exp(-sharpness * centered))) - 1.0f;
                }
                
            case 6: // FenderStyle
            {
                double angle = phase * 2.0 * juce::MathConstants<float>::pi;
                float raw = std::sin(angle) +
                            0.1f * std::sin(2.0 * angle) +
                            0.05f * std::sin(3.0 * angle);
                
                float value = (raw * 0.4f) + 0.5f;
                value = std::pow(value, 1.08f);
                return (value * 2.0f) - 1.0f; // Convert to -1 to 1 for visualizer
            }
            case 7: // WurlitzerStyle
                {
                    double angle = phase * 2.0 * juce::MathConstants<float>::pi;
                    float sineComponent = std::sin(angle);
                    float triangleComponent = 2.0f * std::abs(2.0f * (phase - 0.5f)) - 1.0f;
                    return 0.6f * sineComponent + 0.4f * triangleComponent;
                }
                
            default:
                return 0.0f;
        }
    }
    
    
    void drawWaveform(juce::Graphics& g,
                      const std::vector<std::pair<float, float>>& points,
                      juce::Colour baseColor)
    {
        auto bounds = getLocalBounds().toFloat();
        float centerY = bounds.getCentreY();
        
        // Outer glow
        g.setColour(baseColor.withAlpha(0.15f));
        drawWaveformPath(g, points, 6.0f);
        
        // Middle glow
        g.setColour(baseColor.withAlpha(0.3f));
        drawWaveformPath(g, points, 3.5f);
        
        // Core line
        g.setColour(baseColor.withAlpha(0.95f));
        drawWaveformPath(g, points, 2.0f);
        
        // Bright center
        g.setColour(baseColor.brighter(0.2f).withAlpha(0.8f));
        drawWaveformPath(g, points, 0.5f);
    }

    void drawWaveformPath(juce::Graphics& g, const std::vector<std::pair<float, float>>& points,
                         float thickness)
    {
        for (size_t i = 0; i < points.size() - 1; ++i) {
            g.drawLine(points[i].first, points[i].second,
                      points[i + 1].first, points[i + 1].second,
                      thickness);
        }
    }
    
    int currentWaveform = 0;
    float depth = 1.0f;
    float phaseOffset = 0.0f;
    float rate = 1.0f;
    double currentPhase;
    bool tempoSynced = false;
    double bpm = 120.0;
    int noteDivision = 2; // quarter note by default
    
    bool active = false;

    float crtPhase = 0.0f;
    
    bool waitingForReset = false;
    
    float waveshapeDepth = 0.0f;
    float waveshapeRate = 1.0f;
    int waveshapeWaveform = 0;
    bool waveshapeEnabled = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOVisualizer)
};
