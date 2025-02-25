/*
  ==============================================================================

    Presets.h
    Created: 25 Feb 2025 5:19:59pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

// Presets.h
#pragma once
#include "PluginProcessor.h"

/**
 * Defines and loads factory presets for the Quacker VST plugin.
 * Each category of presets is grouped into its own function for better organization.
 */
namespace QuackerPresets {
    // Load all factory presets
    void loadAllFactoryPresets(QuackerVSTAudioProcessor& processor);
    
    // Load presets by category
    void loadDefaultPreset(QuackerVSTAudioProcessor& processor);
    void loadVintageAmpPresets(QuackerVSTAudioProcessor& processor);
    void loadRhythmicPresets(QuackerVSTAudioProcessor& processor);
    void loadSpecialEffectsPresets(QuackerVSTAudioProcessor& processor);
    void loadSubtleTexturesPresets(QuackerVSTAudioProcessor& processor);
    void loadWaveshapingPresets(QuackerVSTAudioProcessor& processor);
    void loadCreativePresets(QuackerVSTAudioProcessor& processor);
}
