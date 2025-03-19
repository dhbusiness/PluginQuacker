/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

//==============================================================================
QuackerVSTAudioProcessor::QuackerVSTAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
apvts(*this, nullptr, "Parameters", createParameters())
{
    presetManager = std::make_unique<PresetManager>(apvts);
    presetManager->setPresetLoadedCallback([this]() {
        syncParametersAfterPresetLoad();
    });
    QuackerPresets::loadAllFactoryPresets(*this);
    
    // Default BPM that will be used if no host BPM is available
    currentBPM = defaultBPM;
    lastKnownGoodBPM = defaultBPM;
}

juce::AudioProcessorValueTreeState::ParameterLayout QuackerVSTAudioProcessor::createParameters()
{
    // Create parameter groups
    auto lfoGroup = std::make_unique<juce::AudioProcessorParameterGroup>("lfo", "LFO", "|");
    auto tremoloGroup = std::make_unique<juce::AudioProcessorParameterGroup>("tremolo", "Tremolo", "|");
    auto waveshapeGroup = std::make_unique<juce::AudioProcessorParameterGroup>("waveshape", "Waveshaping", "|");
    auto utilityGroup = std::make_unique<juce::AudioProcessorParameterGroup>("utility", "Utility", "|");
    
    // LFO Rate with skew factor for exponential response
    auto rateParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoRate", 1),  // Added version hint
        "LFO Rate",
        juce::NormalisableRange<float>(0.01f, 25.0f, 0.001f, 0.3f),
        1.0f,
        "Hz"
    );
    lfoGroup->addChild(std::move(rateParam));

    // LFO Depth
    auto depthParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoDepth", 1),  // Added version hint
        "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        "%"
    );
    tremoloGroup->addChild(std::move(depthParam));

    // LFO Waveform
    auto waveformParam = std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lfoWaveform", 1),  // Added version hint
        "LFO Waveform",
        juce::StringArray{
            "Sine",
            "Square",
            "Triangle",
            "Sawtooth Up",
            "Sawtooth Down",
            "Soft Square",
            "Fender Style",
            "Wurlitzer Style",
            "Vox Style",
            "Magnatone Style",
            "Pulse Decay",
            "Bouncing Ball",
            "Multi Sine",
            "Optical Style",
            "Twin Peaks",
            "Smooth Random",
            "Guitar Pick",
            "Vintage Chorus",
            "Slow Gear"
        },
        0  // default to Sine
    );
    lfoGroup->addChild(std::move(waveformParam));

    // LFO Sync
    auto syncParam = std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("lfoSync", 1),  // Added version hint
        "LFO Sync",
        false
    );
    lfoGroup->addChild(std::move(syncParam));

    // Note Division
    auto divisionParam = std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lfoNoteDivision", 1),  // Added version hint
        "LFO Note Division",
        juce::StringArray{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" },
        2  // default to Quarter note
    );
    lfoGroup->addChild(std::move(divisionParam));

    // Phase Offset - fixed non-ASCII degree symbol
    auto phaseOffsetParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoPhaseOffset", 1),  // Added version hint
        "LFO Phase Offset",
        juce::NormalisableRange<float>(-180.0f, 180.0f, 1.0f),
        0.0f,
        juce::CharPointer_UTF8(" °")
    );
    lfoGroup->addChild(std::move(phaseOffsetParam));
    
    // Mix Control
    auto mixParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1),  // Added version hint
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f,  // default to 100% wet
        "%"
    );
    utilityGroup->addChild(std::move(mixParam));

    // Add bypass parameter
    auto bypassParam = std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 1),  // Added version hint
        "Bypass",
        false
    );
    utilityGroup->addChild(std::move(bypassParam));

    // Waveshaping parameters
    auto waveshapeRateParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("waveshapeRate", 1),  // Added version hint
        "Waveshape Rate",
        juce::NormalisableRange<float>(0.01f, 25.0f, 0.001f, 0.3f),
        1.0f,
        "Hz"
    );
    waveshapeGroup->addChild(std::move(waveshapeRateParam));

    auto waveshapeDepthParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("waveshapeDepth", 1),  // Added version hint
        "Waveshape Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f,
        "%"
    );
    waveshapeGroup->addChild(std::move(waveshapeDepthParam));

    auto waveshapeWaveformParam = std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("waveshapeWaveform", 1),  // Added version hint
        "Waveshape Waveform",
        juce::StringArray{
            "Sine",
            "Square",
            "Triangle",
            "Sawtooth Up",
            "Sawtooth Down",
            "Soft Square",
            "Fender Style",
            "Wurlitzer Style",
            "Vox Style",
            "Magnatone Style",
            "Pulse Decay",
            "Bouncing Ball",
            "Multi Sine",
            "Optical Style",
            "Twin Peaks",
            "Smooth Random",
            "Guitar Pick",
            "Vintage Chorus",
            "Slow Gear"
        },
        0
    );
    waveshapeGroup->addChild(std::move(waveshapeWaveformParam));
    
    auto waveshapeEnabledParam = std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("waveshapeEnabled", 1),  // Added version hint
        "Waveshape Enabled",
        false
    );
    waveshapeGroup->addChild(std::move(waveshapeEnabledParam));

    // Create the layout with all parameter groups
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::move(lfoGroup),
               std::move(tremoloGroup),
               std::move(waveshapeGroup),
               std::move(utilityGroup));

    return layout;
}


QuackerVSTAudioProcessor::~QuackerVSTAudioProcessor()
{
    // Remove parameter listeners to avoid callbacks after destruction
    for (auto* param : getParameters())
    {
        param->removeListener(this);
    }
    
    // Add this line to clean up static resources
    QuackerVSTAudioProcessorEditor::cleanupStaticResources();
}

//==============================================================================
const juce::String QuackerVSTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QuackerVSTAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool QuackerVSTAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool QuackerVSTAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double QuackerVSTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int QuackerVSTAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int QuackerVSTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QuackerVSTAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String QuackerVSTAudioProcessor::getProgramName (int index)
{
    return {};
}

void QuackerVSTAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void QuackerVSTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSpecs.sampleRate = sampleRate;
    currentSpecs.maximumBlockSize = samplesPerBlock;
    currentSpecs.numChannels = getTotalNumOutputChannels();

    // Initialize with current BPM
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo) && posInfo.bpm > 0.0)
        {
            currentBPM = posInfo.bpm;
            lastKnownGoodBPM = currentBPM;
        }
        else
        {
            // Use default if host doesn't provide valid BPM
            currentBPM = defaultBPM;
            DBG("No valid BPM provided during prepareToPlay, using default: " +
                                    juce::String(defaultBPM));
        }
    }
    else
    {
        // No playhead available (standalone mode)
        currentBPM = defaultBPM;
        DBG("No playhead available during prepareToPlay, using default BPM: " +
                                juce::String(defaultBPM));
    }
    
    // Ensure the LFO has a valid BPM using our safe getter
    lfo.setBPM(getSafeBPM());
    
    // Allocate buffer with alignment for SIMD operations
    lfoValuesBuffer.allocate(samplesPerBlock + 4, true);

    // High-quality DC filter setup - notice the gentler settings to maintain audio quality
    *dcFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate,
        5.0f,    // Gentle cutoff to avoid affecting the audio
        0.707f   // Butterworth Q for optimal flatness
    );
    dcFilter.prepare(currentSpecs);
    
    lfo.setSampleRate(sampleRate);
}

void QuackerVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    dcFilter.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool QuackerVSTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void QuackerVSTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Get playhead info
    bool isPlaying = false;
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    if (auto* playHead = getPlayHead())
    {
        try {
            if (playHead->getCurrentPosition(posInfo))
            {
                isPlaying = posInfo.isPlaying;
                
                // Store the BPM whenever it's valid (non-zero)
                if (posInfo.bpm > 0.0)
                {
                    currentBPM = posInfo.bpm;
                    lastKnownGoodBPM = currentBPM; // Save for later use
                }
                else if (lastKnownGoodBPM > 0.0)
                {
                    // Use last known good BPM if current is invalid
                    currentBPM = lastKnownGoodBPM;
                    DBG("Host provided invalid BPM: " + juce::String(posInfo.bpm) +
                                             ". Using last known good BPM: " + juce::String(lastKnownGoodBPM));
                }
                else
                {
                    // No valid BPM has been seen yet, use default
                    currentBPM = defaultBPM;
                    DBG("Host provided invalid BPM and no history exists. Using default: " +
                                             juce::String(defaultBPM));
                }
                
                currentlyPlaying = isPlaying;
            }
        }
        catch (const std::exception& e) {
            // Protect against any unexpected exceptions when getting position info
            DBG("Exception when getting playhead position: " + juce::String(e.what()));
            currentBPM = lastKnownGoodBPM > 0.0 ? lastKnownGoodBPM : defaultBPM;
        }
    }
    else
    {
        // No playhead available (likely standalone mode)
        if (currentBPM <= 0.0)
            currentBPM = defaultBPM;
        DBG("No playhead available, using BPM: " + juce::String(currentBPM));
    }

    // Final safety check - if we somehow still have an invalid BPM, use default
    if (currentBPM <= 0.0)
    {
        currentBPM = defaultBPM;
        DBG("Emergency BPM fallback activated, using default: " + juce::String(defaultBPM));
    }

    // Update the LFO with our validated BPM value
    lfo.setBPM(getSafeBPM());

    // Check for audio signal
    bool hasSignal = false;
    for (int channel = 0; channel < totalNumInputChannels && !hasSignal; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples() && !hasSignal; ++sample)
        {
            if (std::abs(channelData[sample]) > 0.0001f)
            {
                hasSignal = true;
            }
        }
    }
    
    // Always update audioInputDetected so the visualizer can pick it up
    audioInputDetected = hasSignal;
    
    // Get bypass parameter
    auto* bypassParam = apvts.getRawParameterValue("bypass");
    bool isBypassed = bypassParam->load();

    // If bypassed, just return the dry signal
    if (isBypassed)
    {
        // Update states but don't process audio
        currentlyPlaying = isPlaying;
        lfo.resetPhase(); // Reset LFO when bypassed
        return;
    }
    
    // KEY CHANGE: Allow effect to run even when transport isn't active
    // We'll detect audio activity regardless of playhead state
    bool isActive = hasSignal; // Remove the isPlaying requirement
    
    // We still want to tell the LFO about the transport state for some behaviors
    lfo.updateActiveState(isActive, isPlaying);
    
    // Get parameters from APVTS
    auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
    auto* rateParam = apvts.getRawParameterValue("lfoRate");
    auto* depthParam = apvts.getRawParameterValue("lfoDepth");
    auto* phaseOffsetParam = apvts.getRawParameterValue("lfoPhaseOffset");
    auto* syncParam = apvts.getRawParameterValue("lfoSync");
    auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
    auto* mixParam = apvts.getRawParameterValue("mix");
    float mix = mixParam->load();
    
    // Get waveshaping parameters
    auto* waveshapeRateParam = apvts.getRawParameterValue("waveshapeRate");
    auto* waveshapeDepthParam = apvts.getRawParameterValue("waveshapeDepth");
    auto* waveshapeWaveformParam = apvts.getRawParameterValue("waveshapeWaveform");
    auto* waveshapeEnabledParam = apvts.getRawParameterValue("waveshapeEnabled");

    // Update waveshaping LFO parameters
    lfo.setWaveshapeParameters(
        waveshapeRateParam->load(),
        waveshapeDepthParam->load(),
        static_cast<int>(waveshapeWaveformParam->load()),
        waveshapeEnabledParam->load() > 0.5f
    );

    // Set LFO parameters
    lfo.setWaveform(static_cast<TremoloLFO::Waveform>(static_cast<int>(waveformParam->load())));
    lfo.setDepth(depthParam->load());
    lfo.setPhaseOffset(phaseOffsetParam->load());

    bool isInSync = syncParam->load() > 0.5f;

    if (isInSync != wasInSync) {
        if (isInSync) {
            // Switching TO sync mode
            float currentRate = rateParam->load();
            lfo.storeManualRate(currentRate);
            const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
            double division = divisions[static_cast<int>(divisionParam->load())];
            double syncedFreq = TremoloLFO::bpmToFrequency(currentBPM, division);
            if (syncedFreq > 15.0) {
                syncedFreq = 15.0 + (std::log10(syncedFreq - 14.0) * 5.0);
            }
            syncedFreq = juce::jlimit(0.01, 25.0, syncedFreq);
            if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                rateParameter->setValueNotifyingHost(rateParameter->convertTo0to1(syncedFreq));
            }
            lfo.setSyncMode(true, division);
            lfo.setRate(static_cast<float>(syncedFreq));
        } else {
            // Switching FROM sync mode
            float manualRate = lfo.getLastManualRate();
            if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                rateParameter->setValueNotifyingHost(rateParameter->convertTo0to1(manualRate));
            }
            lfo.setSyncMode(false);
            lfo.setRate(manualRate);
        }
    }
    wasInSync = isInSync;

    // Regular parameter handling
    if (isInSync) {
        try {
            const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
            int divisionIndex = static_cast<int>(divisionParam->load());
            
            // Safety check for division index
            if (divisionIndex < 0 || divisionIndex >= 6) {
                divisionIndex = 2; // Default to quarter note
                DBG("Invalid division index: " + juce::String(divisionIndex) +
                                         ". Using default (1/4 note)");
            }
            
            double division = divisions[divisionIndex];
            
            // Use the safe BPM getter
            double safeBPM = getSafeBPM();
            lfo.setBPM(safeBPM);
            
            // Calculate tempo-synced frequency with protected calculation
            double syncedFreq = TremoloLFO::bpmToFrequency(safeBPM, division);
            
            // Apply high-frequency compression
            if (syncedFreq > 15.0) {
                syncedFreq = 15.0 + (std::log10(std::max(1.0, syncedFreq - 14.0)) * 5.0);
            }
            syncedFreq = juce::jlimit(0.01, 25.0, syncedFreq);
            
            // Set the actual rate in the LFO and update parameter
            lfo.setRate(static_cast<float>(syncedFreq));
            if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                rateParameter->setValueNotifyingHost(rateParameter->convertTo0to1(syncedFreq));
            }
        }
        catch (const std::exception& e) {
            // Handle any calculation errors
            DBG("Error in sync processing: " + juce::String(e.what()));
            
            // Fallback to a safe rate
            lfo.setRate(1.0f);
            if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                rateParameter->setValueNotifyingHost(rateParameter->convertTo0to1(1.0f));
            }
        }
    }
    else {
        // In manual mode
        float manualRate = rateParam->load();
        lfo.setRate(manualRate);
        // Don't store the rate here - we only want to store it when switching TO sync mode
    }

    // KEY CHANGE: Pre-calculate LFO values with high quality - removed isPlaying dependency
    if (hasSignal || lfo.isWaitingForReset())
    {
        for (int i = 0; i < numSamples; ++i)
        {
            lfoValuesBuffer[i] = 0.5f + (lfo.getNextSample() - 0.5f);
        }
    }
    else
    {
        juce::FloatVectorOperations::fill(lfoValuesBuffer, 1.0f, numSamples);
    }

    // Process audio with SIMD operations while maintaining quality
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* dryData = buffer.getReadPointer(channel);
        
        // KEY CHANGE: Apply effect if there's signal or waiting for reset - removed isPlaying dependency
        if (hasSignal || lfo.isWaitingForReset())
        {
            // Store dry signal for mix
            juce::AudioBuffer<float> dryBuffer(1, numSamples);
            dryBuffer.copyFrom(0, 0, dryData, numSamples);

            // Apply modulation
            juce::FloatVectorOperations::multiply(channelData, lfoValuesBuffer, numSamples);

            // Apply mix with precise interpolation
            if (mix < 1.0f)
            {
                juce::FloatVectorOperations::multiply(channelData, mix, numSamples);
                juce::FloatVectorOperations::addWithMultiply(channelData,
                    dryBuffer.getReadPointer(0),
                    1.0f - mix,
                    numSamples);
            }
        }
    }

    // Apply DC filtering
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    dcFilter.process(context);
}


//==============================================================================
bool QuackerVSTAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* QuackerVSTAudioProcessor::createEditor()
{
    return new QuackerVSTAudioProcessorEditor (*this);
}

//==============================================================================

void QuackerVSTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Get the APVTS state
    auto state = apvts.copyState();
    
    // Add the current preset name as a property
    state.setProperty("presetName", presetManager->getDisplayedPresetName(), nullptr);
    
    // Convert to XML and save
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void QuackerVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            // Create a ValueTree from the XML
            juce::ValueTree vt = juce::ValueTree::fromXml(*xmlState);
            
            // Get the preset name before replacing the state
            juce::String savedPresetName = vt.getProperty("presetName", "Default");
            
            // Replace the state (loads all parameters)
            apvts.replaceState(vt);
            
            // Now set the preset name in the PresetManager
            // We need to check if this is an actual preset or just a custom setting
            if (presetManager->loadPreset(savedPresetName))
            {
                // Preset was loaded successfully
            }
            else
            {
                // This is a custom setting, still update the display name
                presetManager->setCustomPresetName(savedPresetName);
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QuackerVSTAudioProcessor();
}

double QuackerVSTAudioProcessor::getCurrentBPM() const {
    return getSafeBPM(); // Use the safe method
}


void QuackerVSTAudioProcessor::applyParametersInOrder() {
    // Get raw parameter values
    auto* syncParam = apvts.getRawParameterValue("lfoSync");
    auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
    auto* rateParam = apvts.getRawParameterValue("lfoRate");
    auto* depthParam = apvts.getRawParameterValue("lfoDepth");
    auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
    auto* phaseOffsetParam = apvts.getRawParameterValue("lfoPhaseOffset");
    
    // First handle sync state to ensure proper rate calculation
    bool isInSync = syncParam->load() > 0.5f;
    
    // Set sync mode first
    lfo.setSyncMode(isInSync, static_cast<int>(divisionParam->load()));
    
    // Then set other parameters
    lfo.setWaveform(static_cast<TremoloLFO::Waveform>(static_cast<int>(waveformParam->load())));
    lfo.setDepth(depthParam->load());
    lfo.setPhaseOffset(phaseOffsetParam->load());
    lfo.setRate(rateParam->load());
}

void QuackerVSTAudioProcessor::syncParametersAfterPresetLoad()
{
    // Force a complete refresh by temporarily flipping wasInSync
    // This ensures the next processBlock call will apply all parameters correctly
    wasInSync = !apvts.getRawParameterValue("lfoSync")->load() > 0.5f;
}

void QuackerVSTAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    // This gets called when hosts automate parameters
    // You can handle specific parameters if needed
    if (auto* param = apvts.getParameter("lfoSync"))
    {
        if (parameterIndex == param->getParameterIndex())
        {
            // Sync parameter changed - may need special handling
            bool isSync = newValue > 0.5f;
            // Your logic here
        }
    }
}
