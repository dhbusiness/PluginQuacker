/*
  ==============================================================================

    LFOVisualizer.h
    Created: 30 Jan 2025 3:21:36pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LFOVisualizer : public juce::Component, public juce::Timer
{
public:
    LFOVisualizer() : currentPhase(0.0)
    {
        startTimerHz(100); // Increase refresh rate for smoother animation
    }

    ~LFOVisualizer() override
    {
        stopTimer();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Fill background
        g.setColour(juce::Colours::black);
        g.fillRect(bounds);

        // Draw grid
        g.setColour(juce::Colours::darkgrey);
        float midY = bounds.getCentreY();
        g.drawHorizontalLine(static_cast<int>(midY), 0.0f, bounds.getWidth());
        
        // Draw moving waveform
        g.setColour(juce::Colour(25, 224, 139));  // Light rose gold for waveform
        juce::Path waveformPath;
        bool pathStarted = false;

        const float pointsPerPixel = 1.0f;
        const int numPoints = static_cast<int>(bounds.getWidth() * pointsPerPixel);

        for (int i = 0; i < numPoints; ++i)
        {
            float x = (static_cast<float>(i) / static_cast<float>(numPoints - 1)) * bounds.getWidth();
            
            // Calculate phase for this point, incorporating the moving phase offset
            float phase = (static_cast<float>(i) / static_cast<float>(numPoints - 1) + currentPhase);
                        
            // Apply phase offset
            phase += phaseOffset / 360.0f;

            // Wrap phase to keep it between 0 and 1
            while (phase >= 1.0f) phase -= 1.0f;
            while (phase < 0.0f) phase += 1.0f;

            float y = bounds.getCentreY();
            float value = calculateWaveformValue(phase, currentWaveform);
            
            // Scale the value by depth and center it
            y -= value * depth * bounds.getHeight() * 0.4f;

            if (!pathStarted)
            {
                waveformPath.startNewSubPath(x, y);
                pathStarted = true;
            }
            else
            {
                waveformPath.lineTo(x, y);
            }
        }

        g.strokePath(waveformPath, juce::PathStrokeType(2.0f));

        // Draw playhead
        g.setColour(juce::Colours::yellow.withAlpha(0.5f));
        float playheadX = currentPhase * bounds.getWidth();
        g.drawVerticalLine(static_cast<int>(playheadX), bounds.getY(), bounds.getBottom());

        // Draw border in dark rose gold
        g.setColour(juce::Colour(120, 80, 75));  // Dark rose gold
        g.drawRect(bounds, 1.0f);

        // Draw rate indicator
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        juce::String rateText;
        if (tempoSynced)
        {
            const char* divisions[] = { "1/1", "1/2", "1/4", "1/8", "1/16" };
            rateText = juce::String(bpm, 1) + " BPM - " + juce::String(divisions[noteDivision]);
        }
        else
        {
            rateText = juce::String(rate, 2) + " Hz";
        }
        g.drawText(rateText, bounds.removeFromTop(20), juce::Justification::centred);
    }

    void resized() override {}


    
    void timerCallback() override
    {
        if (tempoSynced)
        {
            // Calculate phase increment based on BPM and note division
            const double quarterNoteRate = bpm / 60.0; // Quarter notes per second
            const double multipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0 }; // whole, half, quarter, eighth, sixteenth
            double frequencyHz = quarterNoteRate * multipliers[noteDivision] * 2.0; // Added *2.0 to match audio processor
            double phaseIncrement = frequencyHz / 100.0; // 100Hz is our timer frequency
            currentPhase += phaseIncrement;
        }
        else
        {
            // Free-running mode
            currentPhase += rate / 100.0;
        }

        while (currentPhase >= 1.0)
            currentPhase -= 1.0;

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
    
    int currentWaveform = 0;
    float depth = 1.0f;
    float phaseOffset = 0.0f;
    float rate = 1.0f;
    double currentPhase;
    bool tempoSynced = false;
    double bpm = 120.0;
    int noteDivision = 2; // quarter note by default

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOVisualizer)
};
