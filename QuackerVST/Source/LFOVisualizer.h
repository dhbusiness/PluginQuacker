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
        startTimerHz(60); // Increase refresh rate for smoother animation
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
        g.setColour(juce::Colours::cyan);
        juce::Path waveformPath;
        bool pathStarted = false;

        const float pointsPerPixel = 1.0f;
        const int numPoints = static_cast<int>(bounds.getWidth() * pointsPerPixel);

        for (int i = 0; i < numPoints; ++i)
        {
            float x = (static_cast<float>(i) / static_cast<float>(numPoints - 1)) * bounds.getWidth();
            
            // Calculate phase for this point, incorporating the moving phase offset
            float phase = (static_cast<float>(i) / static_cast<float>(numPoints - 1) + currentPhase);
                        
            // Apply phase offset (converting from degrees to normalized phase)
            phase += phaseOffset / 360.0f;

            // Wrap phase to keep it between 0 and 1
            while (phase >= 1.0f) phase -= 1.0f;
            while (phase < 0.0f) phase += 1.0f;

            float y = bounds.getCentreY();
            float value = 0.0f;

            switch (currentWaveform)
            {
                case 0: // Sine
                    value = std::sin(phase * juce::MathConstants<float>::twoPi);
                    break;
                case 1: // Square
                    value = (phase < 0.5f) ? 1.0f : -1.0f;
                    break;
                case 2: // Triangle
                    value = 4.0f * (phase < 0.5f ? phase : (1.0f - phase)) - 1.0f;
                    break;
            }

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

        // Draw playhead (vertical line showing current position)
        g.setColour(juce::Colours::yellow.withAlpha(0.5f));
        float playheadX = currentPhase * bounds.getWidth();
        g.drawVerticalLine(static_cast<int>(playheadX), bounds.getY(), bounds.getBottom());

        // Draw border
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1.0f);

        // Draw rate indicator
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        juce::String rateText;
        if (tempoSynced)
        {
            // Convert note division to text
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
            const double secondsPerBeat = 60.0 / bpm;
            const double multipliers[] = { 4.0, 2.0, 1.0, 0.5, 0.25 }; // whole, half, quarter, eighth, sixteenth
            double cycleLength = secondsPerBeat * multipliers[noteDivision];
            double phaseIncrement = (1.0 / 60.0) / cycleLength; // 60 is our timer frequency
            currentPhase += phaseIncrement;
        }
        else
        {
            // Free-running mode
            currentPhase += rate / 60.0; // 60 is our timer frequency
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
