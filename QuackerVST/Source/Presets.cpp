/*
  ==============================================================================

    Presets.cpp
    Created: 25 Feb 2025 5:19:59pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "Presets.h"

void QuackerPresets::loadAllFactoryPresets(QuackerVSTAudioProcessor& processor) {
    // Clear any previous factory presets
    processor.getPresetManager().clearFactoryPresets();
    
    // Load presets by category
    loadDefaultPreset(processor);
    loadVintageAmpPresets(processor);
    loadRhythmicPresets(processor);
    loadSpecialEffectsPresets(processor);
    loadSubtleTexturesPresets(processor);
    loadWaveshapingPresets(processor);
    loadCreativePresets(processor);
    loadSynthPresets(processor);
    loadGuitarPresets(processor);
    loadVocalPresets(processor);
    loadExperimentalPresets(processor);
    loadAmbiencePresets(processor);
    
    
    
    // Always finish by loading the default preset
    processor.getPresetManager().loadPreset("Default");
}

void QuackerPresets::loadDefaultPreset(QuackerVSTAudioProcessor& processor) {
    // Access the processor's AudioProcessorValueTreeState
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Create the "Default" preset ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(1.0f)); // Default: 1 Hz
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.5f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(2)); // 1/4 note
    
    presetManager.savePreset("Default", "Factory");
}

void QuackerPresets::loadVintageAmpPresets(QuackerVSTAudioProcessor& processor) {
    // Access the processor's AudioProcessorValueTreeState
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Fender Deluxe Tremolo ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.75f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(6)); // Fender Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Fender Deluxe", "Factory/Vintage Amps");
    
    // --- Vox AC30 Tremolo ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.2f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(8)); // Vox Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(10.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.9f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Vox AC30", "Factory/Vintage Amps");
    
    // --- Magnatone Vibrato ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(6.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.65f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(9)); // Magnatone Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(15.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.85f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Magnatone Vibrato", "Factory/Vintage Amps");
    
    // --- Wurlitzer Vibrato ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.5f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.68f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(7)); // Wurlitzer Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(5.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.92f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Wurlitzer Vibrato", "Factory/Vintage Amps");
    
    // --- Princeton Reverb ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(3.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.6f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(6)); // Fender Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.95f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Princeton Reverb", "Factory/Vintage Amps");
    
    // --- Rhodes Piano ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.6f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(7)); // Wurlitzer Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(15.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.85f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Rhodes Piano", "Factory/Vintage Amps");
}

void QuackerPresets::loadRhythmicPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Quarter Note Pulse ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(2)); // 1/4 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(2.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.9f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(1)); // Square
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Quarter Note Pulse", "Factory/Rhythmic");

    // --- Eighth Note Groove ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(3)); // 1/8 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(2)); // Triangle
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Eighth Note Groove", "Factory/Rhythmic");

    // --- Sixteenth Note Chop ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(4)); // 1/16 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(8.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(5)); // Soft Square
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Sixteenth Note Chop", "Factory/Rhythmic");

    // --- Bouncing Ball Rhythm ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(2)); // 1/4 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(2.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.85f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(11)); // Bouncing Ball
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Bouncing Ball Rhythm", "Factory/Rhythmic");

    // --- Twin Peaks Rhythm ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(3)); // 1/8 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.9f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(14)); // Twin Peaks
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Twin Peaks Rhythm", "Factory/Rhythmic");
    
    // --- Dotted Eighth Delay ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(3)); // 1/8 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(16)); // Guitar Pick
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(33.0f)); // Dotted timing effect
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.7f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Dotted Eighth Pattern", "Factory/Rhythmic");
    
    // --- Trap Hi-Hat ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(4)); // 1/16 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(8.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(10)); // Pulse Decay
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(30.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.9f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(2.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.4f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Trap Hi-Hat", "Factory/Rhythmic");
}


void QuackerPresets::loadSpecialEffectsPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Helicopter ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(15.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add waveshaping for additional rotational turbulence
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(2.2f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.4f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Helicopter", "Factory/Special Effects");
    
    // --- Underwater ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(1.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(90.0f)); // 90 degree offset for "wavy" feel
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.85f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add subtle waveshaping for "water current" effect
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.5f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.3f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(12)); // Multi Sine
    presetManager.savePreset("Underwater", "Factory/Special Effects");
    
    // --- Radio Interference ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(7.5f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.6f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(45.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.75f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add waveshaping for "static" effect
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(12.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.35f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Radio Interference", "Factory/Special Effects");
    
    // --- Motor Drive ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(9.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(4)); // Sawtooth Down
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add waveshaping for "motor rev" effect
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(1.2f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.5f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(1)); // Square
    presetManager.savePreset("Motor Drive", "Factory/Special Effects");
    
    // --- Record Scratch ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(20.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.9f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(3)); // Sawtooth Up
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(6.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.6f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Record Scratch", "Factory/Special Effects");
    
    // --- Sci-Fi Teleporter ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(12.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(180.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(24.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.7f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(12)); // Multi Sine
    presetManager.savePreset("Sci-Fi Teleporter", "Factory/Special Effects");
}

void QuackerPresets::loadSubtleTexturesPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Gentle Waves ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.3f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.25f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.7f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Gentle Waves", "Factory/Subtle Textures");
    
    // --- Slow Breath ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.2f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.3f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(18)); // Slow Gear
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.8f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Slow Breath", "Factory/Subtle Textures");
    
    // --- Subtle Shimmer ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.15f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(12)); // Multi Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(45.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.65f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Subtle Shimmer", "Factory/Subtle Textures");
    
    // --- Ambient Movement ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.2f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.6f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Ambient Movement", "Factory/Subtle Textures");
}

void QuackerPresets::loadWaveshapingPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Dual Sine Modulation ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(3.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add sine waveshaping for complex modulation
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.7f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.6f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(0)); // Sine
    presetManager.savePreset("Dual Sine Modulation", "Factory/Waveshaping");
    
    // --- Fast & Slow Combo ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.5f)); // Slow primary rate
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(2)); // Triangle
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add fast square waveshaping
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(8.0f)); // Fast secondary rate
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.4f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(1)); // Square
    presetManager.savePreset("Fast & Slow Combo", "Factory/Waveshaping");
    
    // --- Rhythmic Shaper ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(2)); // 1/4 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(2.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    // Add sync'd waveshaping
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(4.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.5f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(2)); // Triangle
    presetManager.savePreset("Rhythmic Shaper", "Factory/Waveshaping");
    
    // --- Phaser-Like ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.25f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.6f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(2)); // Triangle
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(90.0f)); // 90 degree phase offset
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.8f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add faster waveshaping for the phased effect
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(1.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.3f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(0)); // Sine
    presetManager.savePreset("Phaser-Like", "Factory/Waveshaping");
    
    // --- Psychedelic Warble ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(3.5f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.65f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(30.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.9f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add random waveshaping for unpredictable warbling
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(2.3f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.7f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Psychedelic Warble", "Factory/Waveshaping");
}

void QuackerPresets::loadCreativePresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Classic Tremolo --- (standard preset)
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.0f)); // 5 Hz
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Classic Tremolo", "Factory");
    
    // --- Chorus Emulation ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.4f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(17)); // Vintage Chorus
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(90.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.7f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    // Add subtle waveshaping for more chorus-like complexity
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.3f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.25f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(12)); // Multi Sine
    presetManager.savePreset("Chorus Emulation", "Factory/Creative");
    
    // --- Guitar Swell ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(1)); // 1/2 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(1.5f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.85f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(16)); // Guitar Pick
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Guitar Swell", "Factory/Creative");
    
    // --- Optical Tremolo ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.75f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(13)); // Optical Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.95f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Optical Tremolo", "Factory/Creative");
    
    // --- Stereo Spread ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(180.0f)); // 180 degree phase offset for full stereo spread
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.9f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Stereo Spread", "Factory/Creative");
    
    // --- Vinyl Degradation ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(2.2f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.3f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.5f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.5f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.15f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Vinyl Degradation", "Factory/Creative");
    
    // --- DJ Transitions ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(2)); // 1/4 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(2.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.8f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(3)); // Sawtooth Up
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("DJ Transitions", "Factory/Creative");
}


// New Synth-Focused Presets
void QuackerPresets::loadSynthPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Pad Breathing ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.3f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.4f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(18)); // Slow Gear
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.85f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.1f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.2f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(0)); // Sine
    presetManager.savePreset("Pad Breathing", "Factory/Synth");
    
    // --- Acid Wobble ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(3)); // 1/8 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.9f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(4)); // Sawtooth Down
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(2.5f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.4f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(5)); // Soft Square
    presetManager.savePreset("Acid Wobble", "Factory/Synth");
    
    // --- Analog Drift ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.05f)); // Very slow
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.15f); // Subtle
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.7f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.02f)); // Super slow
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.1f); // Very subtle
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Analog Drift", "Factory/Synth");
    
    // --- Progressive Trance Gate ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(4)); // 1/16 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(8.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(1)); // Square
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(45.0f)); // Slight offset for rhythmic feel
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Progressive Trance Gate", "Factory/Synth");
    
    // --- Arpeggiator Helper ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(4)); // 1/16 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(8.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.6f); // Not fully cutting notes
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(16)); // Guitar Pick
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.85f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Arpeggiator Helper", "Factory/Synth");
}

// Guitar-Focused Presets
void QuackerPresets::loadGuitarPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Surf Rock ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(7.2f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.85f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(6)); // Fender Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Surf Rock", "Factory/Guitar");
    
    // --- Rockabilly Slapback ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(2)); // Triangle
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.9f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(1.2f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.3f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(7)); // Wurlitzer Style
    presetManager.savePreset("Rockabilly Slapback", "Factory/Guitar");
    
    // --- Blues Driver ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(3.5f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.6f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(13)); // Optical Style
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.85f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Blues Driver", "Factory/Guitar");
    
    // --- Floyd Pulse ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.8f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.5f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.75f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.4f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.25f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(0)); // Sine
    presetManager.savePreset("Floyd Pulse", "Factory/Guitar");
    
    // --- Finger Tapper ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(2.5f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.95f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(16)); // Guitar Pick
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Finger Tapper", "Factory/Guitar");
}

// Vocal Processing Presets
void QuackerPresets::loadVocalPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Vocal Chop ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(3)); // 1/8 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.9f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(1)); // Square
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Vocal Chop", "Factory/Vocal");
    
    // --- Glottal Vibrato ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(5.5f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.4f); // Subtle
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.7f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Glottal Vibrato", "Factory/Vocal");
    
    // --- Radio Voice ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(8.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.35f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.6f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(15.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.2f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Radio Voice", "Factory/Vocal");
    
    // --- Backing Vocals ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.85f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.3f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(18)); // Slow Gear
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(180.0f)); // Full phase offset for stereo spreading
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.55f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Backing Vocals", "Factory/Vocal");
}

// Experimental Presets
void QuackerPresets::loadExperimentalPresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Broken Circuit ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(12.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(1.0f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(1)); // Square
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(30.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(22.0f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.5f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Broken Circuit", "Factory/Experimental");
    
    // --- Quantum Fluctuations ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.02f)); // Extremely slow
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.8f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(15.0f)); // Fast
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.3f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Quantum Fluctuations", "Factory/Experimental");
    
    // --- Poly-Rhythmic Chaos ---
    // First, set sync mode
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(true);
    // Then set division
    if (auto* divisionParam = apvts.getParameter("lfoNoteDivision"))
        divisionParam->setValueNotifyingHost(divisionParam->convertTo0to1(3)); // 1/8 note
    // THEN set other parameters
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(4.0f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.9f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(2)); // Triangle
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(1.0f);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(5.33f)); // Non-integer ratio for polyrhythmic effect
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.5f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(2)); // Triangle
    presetManager.savePreset("Poly-Rhythmic Chaos", "Factory/Experimental");
    
    // --- Digital Deconstruction ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(7.7f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.85f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(3)); // Sawtooth Up
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(15.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.9f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(1.7f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.6f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(4)); // Sawtooth Down
    presetManager.savePreset("Digital Deconstruction", "Factory/Experimental");
    
    // --- Probability Waves ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(3.3f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.75f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(90.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.85f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(6.5f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.4f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(12)); // Multi Sine
    presetManager.savePreset("Probability Waves", "Factory/Experimental");
}

// Ambience and Soundscapes Presets
void QuackerPresets::loadAmbiencePresets(QuackerVSTAudioProcessor& processor) {
    auto& apvts = processor.apvts;
    auto& presetManager = processor.getPresetManager();
    
    // --- Ocean Waves ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.25f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.45f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(0)); // Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.7f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.7f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.3f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Ocean Waves", "Factory/Ambience");
    
    // --- Wind Through Trees ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.4f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.35f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(15)); // Smooth Random
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(45.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.6f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(1.2f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.25f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(15)); // Smooth Random
    presetManager.savePreset("Wind Through Trees", "Factory/Ambience");
    
    // --- Distant Thunder ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.15f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.6f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(10)); // Pulse Decay
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.8f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Distant Thunder", "Factory/Ambience");
    
    // --- Heartbeat ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(1.2f));
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.7f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(14)); // Twin Peaks
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(0.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.9f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(false);
    presetManager.savePreset("Heartbeat", "Factory/Ambience");
    
    // --- Aurora Borealis ---
    if (auto* rateParam = apvts.getParameter("lfoRate"))
        rateParam->setValueNotifyingHost(rateParam->convertTo0to1(0.08f)); // Very slow
    if (auto* depthParam = apvts.getParameter("lfoDepth"))
        depthParam->setValueNotifyingHost(0.5f);
    if (auto* waveformParam = apvts.getParameter("lfoWaveform"))
        waveformParam->setValueNotifyingHost(waveformParam->convertTo0to1(12)); // Multi Sine
    if (auto* phaseOffsetParam = apvts.getParameter("lfoPhaseOffset"))
        phaseOffsetParam->setValueNotifyingHost(phaseOffsetParam->convertTo0to1(90.0f));
    if (auto* mixParam = apvts.getParameter("mix"))
        mixParam->setValueNotifyingHost(0.65f);
    if (auto* syncParam = apvts.getParameter("lfoSync"))
        syncParam->setValueNotifyingHost(false);
    if (auto* waveshapeEnabledParam = apvts.getParameter("waveshapeEnabled"))
        waveshapeEnabledParam->setValueNotifyingHost(true);
    if (auto* waveshapeRateParam = apvts.getParameter("waveshapeRate"))
        waveshapeRateParam->setValueNotifyingHost(waveshapeRateParam->convertTo0to1(0.2f));
    if (auto* waveshapeDepthParam = apvts.getParameter("waveshapeDepth"))
        waveshapeDepthParam->setValueNotifyingHost(0.4f);
    if (auto* waveshapeWaveformParam = apvts.getParameter("waveshapeWaveform"))
        waveshapeWaveformParam->setValueNotifyingHost(waveshapeWaveformParam->convertTo0to1(0)); // Sine
    presetManager.savePreset("Aurora Borealis", "Factory/Ambience");
}





    
