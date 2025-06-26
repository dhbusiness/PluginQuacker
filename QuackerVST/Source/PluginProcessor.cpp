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
    try {
        presetManager = std::make_unique<PresetManager>(apvts);
        presetManager->setPresetLoadedCallback([this]() {
            syncParametersAfterPresetLoad();
        });
        
        QuackerPresets::loadAllFactoryPresets(*this);
    }
    catch (const std::exception& e) {
        reportError(ProcessorError::PresetLoadFailed,
                   "Failed to initialize preset manager: " + juce::String(e.what()));
    }
    
    // Initialize with safe defaults
    currentBPM = defaultBPM;
    lastKnownGoodBPM = defaultBPM;
    lfoBufferSize = 0;
}

QuackerVSTAudioProcessor::~QuackerVSTAudioProcessor()
{
    try {
        // Safely remove parameter listeners
        for (auto* param : getParameters()) {
            if (param != nullptr) {
                param->removeListener(this);
            }
        }
        
        // Clean up static resources
        QuackerVSTAudioProcessorEditor::cleanupStaticResources();
    }
    catch (...) {
        // Destructors shouldn't throw
    }
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
    return 1;
}

int QuackerVSTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QuackerVSTAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String QuackerVSTAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void QuackerVSTAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void QuackerVSTAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Validate audio specifications
    if (!validateAudioSpecs(sampleRate, samplesPerBlock)) {
        reportError(ProcessorError::InvalidSampleRate,
                   "Invalid audio specifications: SR=" + juce::String(sampleRate) +
                   " Block=" + juce::String(samplesPerBlock));
        return;
    }

    currentSpecs.sampleRate = sampleRate;
    currentSpecs.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    currentSpecs.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    // Initialize BPM with error handling
    if (auto* playHead = getPlayHead()) {
        try {
            juce::AudioPlayHead::CurrentPositionInfo posInfo;
            if (playHead->getCurrentPosition(posInfo) && posInfo.bpm > 0.0) {
                currentBPM = juce::jlimit(minValidBPM, maxValidBPM, posInfo.bpm);
                lastKnownGoodBPM = currentBPM.load();
            }
        }
        catch (...) {
            DBG("Exception getting playhead position, using default BPM");
            currentBPM = defaultBPM;
        }
    } else {
        currentBPM = defaultBPM;
        DBG("No playhead available, using default BPM: " + juce::String(defaultBPM));
    }
    
    // Set LFO parameters safely
    auto lfoError = lfo.setSampleRate(sampleRate);
    if (lfoError != TremoloLFO::ErrorCode::None) {
        reportError(ProcessorError::ParameterError, "Failed to set LFO sample rate");
    }
    
    lfoError = lfo.setBPM(getSafeBPM());
    if (lfoError != TremoloLFO::ErrorCode::None) {
        reportError(ProcessorError::InvalidBPM, "Failed to set LFO BPM");
    }
    
    // Allocate LFO buffer with error handling
    if (!allocateLFOBuffer(samplesPerBlock)) {
        reportError(ProcessorError::BufferAllocationFailed,
                   "Failed to allocate LFO buffer for " + juce::String(samplesPerBlock) + " samples");
        return;
    }

    // Initialize DC filter with error handling
    try {
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, 5.0f, 0.707f);
        
        if (coefficients != nullptr) {
            *dcFilter.state = *coefficients;
            dcFilter.prepare(currentSpecs);
        } else {
            reportError(ProcessorError::DCFilterInitFailed, "Failed to create DC filter coefficients");
        }
    }
    catch (const std::exception& e) {
        reportError(ProcessorError::DCFilterInitFailed,
                   "DC filter initialization error: " + juce::String(e.what()));
    }
}

void QuackerVSTAudioProcessor::releaseResources()
{
    try {
        dcFilter.reset();
        lfoValuesBuffer.free();
        lfoBufferSize = 0;
    }
    catch (...) {
        // Fail silently during cleanup
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool QuackerVSTAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
  #else
    // Validate channel configuration
    auto mainOut = layouts.getMainOutputChannelSet();
    auto mainIn = layouts.getMainInputChannelSet();
    
    if (mainOut != juce::AudioChannelSet::mono() &&
        mainOut != juce::AudioChannelSet::stereo()) {
        return false;
    }

   #if ! JucePlugin_IsSynth
    if (mainOut != mainIn) {
        return false;
    }
   #endif

    return true;
  #endif
}
#endif

void QuackerVSTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);
    
    // Validate buffer
    if (buffer.getNumSamples() == 0 || buffer.getNumSamples() > maxBlockSize) {
        DBG("Invalid buffer size: " + juce::String(buffer.getNumSamples()));
        return;
    }
    
    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);
    
    // Ensure LFO buffer is large enough
    if (lfoBufferSize < static_cast<size_t>(numSamples + 4)) {
        if (!allocateLFOBuffer(numSamples)) {
            // Process without modulation if allocation fails
            return;
        }
    }
    
    // Get playhead info safely
    bool isPlaying = false;
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    
    if (auto* playHead = getPlayHead()) {
        try {
            if (playHead->getCurrentPosition(posInfo)) {
                isPlaying = posInfo.isPlaying;
                
                // Validate and store BPM
                if (posInfo.bpm > 0.0 && posInfo.bpm <= maxValidBPM) {
                    currentBPM = posInfo.bpm;
                    lastKnownGoodBPM = currentBPM.load();
                } else if (lastKnownGoodBPM > 0.0) {
                    currentBPM = lastKnownGoodBPM.load();
                } else {
                    currentBPM = defaultBPM;
                }
                
                currentlyPlaying = isPlaying;
            }
        }
        catch (...) {
            DBG("Exception getting playhead position");
            currentBPM = getSafeBPM();
        }
    }

    // Update LFO BPM safely
    lfo.setBPM(getSafeBPM());

    // Check for audio signal
    bool hasSignal = false;
    for (int channel = 0; channel < totalNumInputChannels && !hasSignal; ++channel) {
        auto* channelData = buffer.getReadPointer(channel);
        for (int sample = 0; sample < numSamples && !hasSignal; ++sample) {
            if (std::abs(channelData[sample]) > audioDetectionThreshold) {
                hasSignal = true;
            }
        }
    }
    
    audioInputDetected = hasSignal;
    
    // Get bypass state safely
    auto* bypassParam = apvts.getRawParameterValue("bypass");
    bool isBypassed = bypassParam && bypassParam->load() > 0.5f;
    
    if (isBypassed) {
        currentlyPlaying = isPlaying;
        lfo.resetPhase();
        return;
    }
    
    // Process with LFO
    bool isActive = hasSignal;
    lfo.updateActiveState(isActive, isPlaying);
    
    // Process parameter updates safely
    processParameterUpdates();
    
    // Get mix parameter
    auto* mixParam = apvts.getRawParameterValue("mix");
    float mix = mixParam ? mixParam->load() : 1.0f;
    
    // Generate LFO values
    if (hasSignal || lfo.isWaitingForReset()) {
        for (int i = 0; i < numSamples; ++i) {
            lfoValuesBuffer[i] = lfo.getNextSample();
        }
    } else {
        juce::FloatVectorOperations::fill(lfoValuesBuffer, 1.0f, numSamples);
    }
    
    // Process audio channels
    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        
        if (hasSignal || lfo.isWaitingForReset()) {
            // Store dry signal if needed
            if (mix < 1.0f) {
                juce::AudioBuffer<float> dryBuffer(1, numSamples);
                dryBuffer.copyFrom(0, 0, channelData, numSamples);
                
                // Apply modulation
                juce::FloatVectorOperations::multiply(channelData, lfoValuesBuffer, numSamples);
                
                // Apply mix
                juce::FloatVectorOperations::multiply(channelData, mix, numSamples);
                juce::FloatVectorOperations::addWithMultiply(channelData,
                    dryBuffer.getReadPointer(0), 1.0f - mix, numSamples);
            } else {
                // Full wet signal
                juce::FloatVectorOperations::multiply(channelData, lfoValuesBuffer, numSamples);
            }
        }
    }

    // Apply DC filtering safely
    try {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        dcFilter.process(context);
    }
    catch (...) {
        DBG("DC filter processing error");
    }
}

//==============================================================================
bool QuackerVSTAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* QuackerVSTAudioProcessor::createEditor()
{
    try {
        return new QuackerVSTAudioProcessorEditor(*this);
    }
    catch (const std::exception& e) {
        DBG("Failed to create editor: " + juce::String(e.what()));
        return nullptr;
    }
}

//==============================================================================
void QuackerVSTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    try {
        auto state = apvts.copyState();
        
        if (presetManager) {
            state.setProperty("presetName", presetManager->getDisplayedPresetName(), nullptr);
        }
        
        if (auto xml = state.createXml()) {
            copyXmlToBinary(*xml, destData);
        }
    }
    catch (const std::exception& e) {
        reportError(ProcessorError::PresetLoadFailed,
                   "Failed to save state: " + juce::String(e.what()));
    }
}

void QuackerVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    try {
        if (auto xmlState = getXmlFromBinary(data, sizeInBytes)) {
            if (xmlState->hasTagName(apvts.state.getType())) {
                juce::ValueTree vt = juce::ValueTree::fromXml(*xmlState);
                
                juce::String savedPresetName = vt.getProperty("presetName", "Default");
                
                apvts.replaceState(vt);
                
                if (presetManager) {
                    if (!presetManager->loadPreset(savedPresetName)) {
                        presetManager->setCustomPresetName(savedPresetName);
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        reportError(ProcessorError::PresetLoadFailed,
                   "Failed to restore state: " + juce::String(e.what()));
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout QuackerVSTAudioProcessor::createParameters()
{
    // Create parameter groups with error handling
    auto lfoGroup = std::make_unique<juce::AudioProcessorParameterGroup>("lfo", "LFO", "|");
    auto tremoloGroup = std::make_unique<juce::AudioProcessorParameterGroup>("tremolo", "Tremolo", "|");
    auto waveshapeGroup = std::make_unique<juce::AudioProcessorParameterGroup>("waveshape", "Waveshaping", "|");
    auto utilityGroup = std::make_unique<juce::AudioProcessorParameterGroup>("utility", "Utility", "|");
    
    // LFO Rate with proper range validation
    auto rateParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoRate", 1),
        "LFO Rate",
        juce::NormalisableRange<float>(0.01f, 25.0f, 0.001f, 0.3f),
        1.0f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2) + " Hz"; }, // Keep suffix here
        [](const juce::String& text) {
            return juce::jlimit(0.01f, 25.0f, text.getFloatValue());
        }
    );
    lfoGroup->addChild(std::move(rateParam));

    // LFO Depth
    auto depthParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoDepth", 1),
        "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(int(value * 100)) + "%"; }, // Keep suffix here
        [](const juce::String& text) {
            return juce::jlimit(0.0f, 1.0f, text.getFloatValue() / 100.0f);
        }
    );
    tremoloGroup->addChild(std::move(depthParam));

    // LFO Waveform with bounds checking
    auto waveformParam = std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lfoWaveform", 1),
        "LFO Waveform",
        juce::StringArray{
            "Sine", "Square", "Triangle", "Sawtooth Up", "Sawtooth Down",
            "Soft Square", "Fender Style", "Wurlitzer Style", "Vox Style",
            "Magnatone Style", "Pulse Decay", "Bouncing Ball", "Multi Sine",
            "Optical Style", "Twin Peaks", "Smooth Random", "Guitar Pick",
            "Vintage Chorus", "Slow Gear"
        },
        0
    );
    lfoGroup->addChild(std::move(waveformParam));

    // LFO Sync
    auto syncParam = std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("lfoSync", 1),
        "LFO Sync",
        false
    );
    lfoGroup->addChild(std::move(syncParam));

    // Note Division
    auto divisionParam = std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lfoNoteDivision", 1),
        "LFO Note Division",
        juce::StringArray{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" },
        2  // default to 1/4 note
    );
    lfoGroup->addChild(std::move(divisionParam));

    // Phase Offset with validation
    auto phaseOffsetParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lfoPhaseOffset", 1),
        "LFO Phase Offset",
        juce::NormalisableRange<float>(-180.0f, 180.0f, 1.0f),
        0.0f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(int(value)) + juce::String(juce::CharPointer_UTF8("°")); }, // Keep suffix here
        [](const juce::String& text) {
            return juce::jlimit(-180.0f, 180.0f, text.getFloatValue());
        }
    );
    lfoGroup->addChild(std::move(phaseOffsetParam));
    
    // Mix Control
    auto mixParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1),
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(int(value * 100)) + "%"; }, // Keep suffix here
        [](const juce::String& text) {
            return juce::jlimit(0.0f, 1.0f, text.getFloatValue() / 100.0f);
        }
    );
    utilityGroup->addChild(std::move(mixParam));

    // Bypass parameter
    auto bypassParam = std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 1),
        "Bypass",
        false
    );
    utilityGroup->addChild(std::move(bypassParam));

    // Waveshaping parameters with validation
    auto waveshapeRateParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("waveshapeRate", 1),
        "Waveshape Rate",
        juce::NormalisableRange<float>(0.01f, 25.0f, 0.001f, 0.3f),
        1.0f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2) + " Hz"; }, // Keep suffix here
        [](const juce::String& text) {
            return juce::jlimit(0.01f, 25.0f, text.getFloatValue());
        }
    );
    waveshapeGroup->addChild(std::move(waveshapeRateParam));
    
    auto waveshapeDepthParam = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("waveshapeDepth", 1),
        "Waveshape Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(int(value * 100)) + "%"; }, // Keep suffix here
        [](const juce::String& text) {
            return juce::jlimit(0.0f, 1.0f, text.getFloatValue() / 100.0f);
        }
    );
    waveshapeGroup->addChild(std::move(waveshapeDepthParam));

    auto waveshapeWaveformParam = std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("waveshapeWaveform", 1),
        "Waveshape Waveform",
        juce::StringArray{
            "Sine", "Square", "Triangle", "Sawtooth Up", "Sawtooth Down",
            "Soft Square", "Fender Style", "Wurlitzer Style", "Vox Style",
            "Magnatone Style", "Pulse Decay", "Bouncing Ball", "Multi Sine",
            "Optical Style", "Twin Peaks", "Smooth Random", "Guitar Pick",
            "Vintage Chorus", "Slow Gear"
        },
        0
    );
    waveshapeGroup->addChild(std::move(waveshapeWaveformParam));
    
    auto waveshapeEnabledParam = std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("waveshapeEnabled", 1),
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

void QuackerVSTAudioProcessor::applyParametersInOrder()
{
    try {
        // Get parameters safely
        auto* syncParam = apvts.getRawParameterValue("lfoSync");
        auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
        auto* rateParam = apvts.getRawParameterValue("lfoRate");
        auto* depthParam = apvts.getRawParameterValue("lfoDepth");
        auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
        auto* phaseOffsetParam = apvts.getRawParameterValue("lfoPhaseOffset");
        
        if (!syncParam || !divisionParam || !rateParam || !depthParam || !waveformParam || !phaseOffsetParam) {
            reportError(ProcessorError::ParameterError, "Missing required parameters");
            return;
        }
        
        bool isInSync = syncParam->load() > 0.5f;
        
        lfo.setSyncMode(isInSync, static_cast<int>(divisionParam->load()));
        lfo.setWaveform(static_cast<TremoloLFO::Waveform>(
            static_cast<int>(waveformParam->load())));
        lfo.setDepth(depthParam->load());
        lfo.setPhaseOffset(phaseOffsetParam->load());
        lfo.setRate(rateParam->load());
    }
    catch (...) {
        reportError(ProcessorError::ParameterError, "Failed to apply parameters");
    }
}

void QuackerVSTAudioProcessor::syncParametersAfterPresetLoad()
{
    auto* syncParam = apvts.getRawParameterValue("lfoSync");
    if (syncParam) {
        wasInSync = !(syncParam->load() > 0.5f);
    }
}

void QuackerVSTAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    juce::ignoreUnused(parameterIndex, newValue);
    // Handle parameter changes if needed
}

// Helper method implementations
void QuackerVSTAudioProcessor::reportError(ProcessorError::Type type, const juce::String& message) noexcept
{
    const juce::ScopedLock sl(errorLock);
    lastError.type = type;
    lastError.message = message;
    lastError.timestamp = juce::Time::getCurrentTime();
    
    #if JUCE_DEBUG
    DBG("QuackerVST Error: " + message);
    #endif
}

bool QuackerVSTAudioProcessor::validateAudioSpecs(double sampleRate, int samplesPerBlock) const noexcept
{
    return sampleRate >= 8000.0 && sampleRate <= 384000.0 &&
           samplesPerBlock > 0 && samplesPerBlock <= maxBlockSize;
}

bool QuackerVSTAudioProcessor::allocateLFOBuffer(int samplesPerBlock) noexcept
{
    try {
        lfoValuesBuffer.allocate(samplesPerBlock + 4, true);
        lfoBufferSize = samplesPerBlock + 4;
        return true;
    }
    catch (...) {
        lfoBufferSize = 0;
        return false;
    }
}

void QuackerVSTAudioProcessor::processParameterUpdates() noexcept
{
    try {
        // Get parameters safely
        auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
        auto* depthParam = apvts.getRawParameterValue("lfoDepth");
        auto* phaseOffsetParam = apvts.getRawParameterValue("lfoPhaseOffset");
        auto* syncParam = apvts.getRawParameterValue("lfoSync");
        auto* rateParam = apvts.getRawParameterValue("lfoRate");
        auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
        
        // Check all parameters exist
        if (!waveformParam || !depthParam || !phaseOffsetParam ||
            !syncParam || !rateParam || !divisionParam) {
            return;
        }
        
        float waveform = waveformParam->load();
        float depth = depthParam->load();
        float phaseOffset = phaseOffsetParam->load();
        bool isInSync = syncParam->load() > 0.5f;
        float rate = rateParam->load();
        int division = static_cast<int>(divisionParam->load());
        
        // Update waveshaping
        auto* waveshapeRateParam = apvts.getRawParameterValue("waveshapeRate");
        auto* waveshapeDepthParam = apvts.getRawParameterValue("waveshapeDepth");
        auto* waveshapeWaveformParam = apvts.getRawParameterValue("waveshapeWaveform");
        auto* waveshapeEnabledParam = apvts.getRawParameterValue("waveshapeEnabled");
        
        if (waveshapeRateParam && waveshapeDepthParam &&
            waveshapeWaveformParam && waveshapeEnabledParam) {
            
            float waveshapeRate = waveshapeRateParam->load();
            float waveshapeDepth = waveshapeDepthParam->load();
            int waveshapeWaveform = static_cast<int>(waveshapeWaveformParam->load());
            bool waveshapeEnabled = waveshapeEnabledParam->load() > 0.5f;
            
            lfo.setWaveshapeParameters(waveshapeRate, waveshapeDepth,
                                     waveshapeWaveform, waveshapeEnabled);
        }
        
        // Set LFO parameters
        lfo.setWaveform(static_cast<TremoloLFO::Waveform>(static_cast<int>(waveform)));
        lfo.setDepth(depth);
        lfo.setPhaseOffset(phaseOffset);
        
        // Handle sync mode changes
        if (isInSync != wasInSync) {
            if (isInSync) {
                float currentRate = rate;
                lfo.storeManualRate(currentRate);
                
                const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
                division = juce::jlimit(0, 5, division);
                
                double syncedFreq = TremoloLFO::bpmToFrequency(getSafeBPM(), divisions[division]);
                syncedFreq = juce::jlimit(0.01, 25.0, syncedFreq);
                
                if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                    rateParameter->setValueNotifyingHost(
                        rateParameter->convertTo0to1(static_cast<float>(syncedFreq)));
                }
                
                lfo.setSyncMode(true, divisions[division]);
                lfo.setRate(static_cast<float>(syncedFreq));
            } else {
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
            const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
            division = juce::jlimit(0, 5, division);
            
            double syncedFreq = TremoloLFO::bpmToFrequency(getSafeBPM(), divisions[division]);
            syncedFreq = juce::jlimit(0.01, 25.0, syncedFreq);
            
            lfo.setRate(static_cast<float>(syncedFreq));
            
            if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                rateParameter->setValueNotifyingHost(
                    rateParameter->convertTo0to1(static_cast<float>(syncedFreq)));
            }
        } else {
            lfo.setRate(rate);
        }
    }
    catch (...) {
        // Fail silently in real-time context
    }
}

bool QuackerVSTAudioProcessor::validateParameterValue(const juce::String& paramID, float value) const noexcept
{
    if (paramID == "lfoRate" || paramID == "waveshapeRate") {
        return value >= 0.01f && value <= 25.0f;
    }
    else if (paramID == "lfoDepth" || paramID == "waveshapeDepth" || paramID == "mix") {
        return value >= 0.0f && value <= 1.0f;
    }
    else if (paramID == "lfoPhaseOffset") {
        return value >= -180.0f && value <= 180.0f;
    }
    else if (paramID == "lfoWaveform" || paramID == "waveshapeWaveform") {
        return value >= 0.0f && value < 19.0f; // 19 waveforms
    }
    else if (paramID == "lfoNoteDivision") {
        return value >= 0.0f && value < 6.0f; // 6 divisions
    }
    
    return true; // Default to valid for unknown parameters
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    try {
        return new QuackerVSTAudioProcessor();
    }
    catch (...) {
        return nullptr;
    }
}
