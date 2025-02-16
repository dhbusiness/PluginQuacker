/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
}

juce::AudioProcessorValueTreeState::ParameterLayout QuackerVSTAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // LFO Rate
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoRate",           // parameterID
        "LFO Rate",         // parameter name
        0.01f,              // minimum value
        25.0f,               // maximum value
        1.0f               // default value
    ));

    // LFO Depth
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoDepth",
        "LFO Depth",
        0.0f,
        1.0f,
        0.5f
    ));

    // LFO Waveform
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "lfoWaveform",
        "LFO Waveform",
        juce::StringArray{
            "Sine",
            "Square",
            "Triangle",
            "Sawtooth Up",
            "Sawtooth Down",
            "Soft Square",
            "Fender Style",
            "Wurlitzer Style"
        },
        0  // default to Sine
    ));

    // LFO Sync
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "lfoSync",
        "LFO Sync",
        false
    ));

    // Note Division
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "lfoNoteDivision",
        "LFO Note Division",
        juce::StringArray{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" },
        2  // default to Quarter note
    ));

    // Phase Offset
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoPhaseOffset",
        "LFO Phase Offset",
        -180.0f,
        180.0f,
        0.0f
    ));
    
    // Mix Control
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix",
        "Mix",
        0.0f,
        1.0f,
        1.0f  // default to 100% wet
    ));

    
    // Add bypass parameter
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "bypass",
        "Bypass",
        false
    ));


    return { params.begin(), params.end() };
}


QuackerVSTAudioProcessor::~QuackerVSTAudioProcessor()
{
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
        if (playHead->getCurrentPosition(posInfo))
        {
            currentBPM = posInfo.bpm;
            lfo.setBPM(currentBPM);
        }
    }
    
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
    const int numSamples = buffer.getNumSamples();  // Added this declaration
    
    // Get playhead info
    bool isPlaying = false;
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    if (auto* playHead = getPlayHead())
    {
        if (playHead->getCurrentPosition(posInfo))
        {
            isPlaying = posInfo.isPlaying;
            currentBPM = posInfo.bpm;
            currentlyPlaying = isPlaying;
        }
    }

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

    // Get bypass parameter
    auto* bypassParam = apvts.getRawParameterValue("bypass");
    bool isBypassed = bypassParam->load();

    // If bypassed, just return the dry signal
    if (isBypassed)
    {
        // Update states but don't process audio
        audioInputDetected = hasSignal;
        currentlyPlaying = isPlaying;
        lfo.resetPhase(); // Reset LFO when bypassed
        return;
    }
    
    // Update the member variable here
    audioInputDetected = hasSignal;
    bool isActive = isPlaying && hasSignal;
    lfo.updateActiveState(isActive, isPlaying); // Pass both states
    

    
    // Get parameters from APVTS
    auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
    auto* rateParam = apvts.getRawParameterValue("lfoRate");
    auto* depthParam = apvts.getRawParameterValue("lfoDepth");
    auto* phaseOffsetParam = apvts.getRawParameterValue("lfoPhaseOffset");
    auto* syncParam = apvts.getRawParameterValue("lfoSync");
    auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
    auto* mixParam = apvts.getRawParameterValue("mix");
    float mix = mixParam->load();
    
    static bool wasInSync = false;  // Track previous sync state
    bool isInSync = syncParam->load() > 0.5f;

    // Set LFO parameters
    lfo.setWaveform(static_cast<TremoloLFO::Waveform>(static_cast<int>(waveformParam->load())));
    lfo.setDepth(depthParam->load());
    lfo.setPhaseOffset(phaseOffsetParam->load());

    // Handle sync state changes
    if (isInSync != wasInSync) {
        if (isInSync) {
            // Switching to sync mode - current rate will be stored in LFO
            const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
            double division = divisions[static_cast<int>(divisionParam->load())];
            
            double syncedFreq = TremoloLFO::bpmToFrequency(currentBPM, division);
            
            // Apply compression for high frequencies
            if (syncedFreq > 15.0) {
                syncedFreq = 15.0 + (std::log10(syncedFreq - 14.0) * 5.0);
            }
            syncedFreq = juce::jlimit(0.01, 25.0, syncedFreq);
            
            // Update the rate parameter
            if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                float normalizedValue = (syncedFreq - 0.01f) / (25.0f - 0.01f);
                rateParameter->setValueNotifyingHost(normalizedValue);
            }
            
            lfo.setSyncMode(true, division);
        } else {
            // Switching back to manual mode - restore previous rate
            float manualRate = lfo.getLastManualRate();
            
            // Update the rate parameter
            if (auto* rateParameter = apvts.getParameter("lfoRate")) {
                float normalizedValue = (manualRate - 0.01f) / (25.0f - 0.01f);
                rateParameter->setValueNotifyingHost(normalizedValue);
            }
            
            lfo.setSyncMode(false);
            lfo.setRate(manualRate);
        }
    }
    
    wasInSync = isInSync;

    // Regular sync handling for when we're already in sync mode
    if (isInSync) {
        const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
        double division = divisions[static_cast<int>(divisionParam->load())];
        
        // Update LFO with current BPM
        lfo.setBPM(currentBPM);
        
        double syncedFreq = TremoloLFO::bpmToFrequency(currentBPM, division);
        
        if (syncedFreq > 15.0) {
            syncedFreq = 15.0 + (std::log10(syncedFreq - 14.0) * 5.0);
        }
        syncedFreq = juce::jlimit(0.01, 25.0, syncedFreq);
        
        if (auto* rateParameter = apvts.getParameter("lfoRate")) {
            float normalizedValue = (syncedFreq - 0.01f) / (25.0f - 0.01f);
            rateParameter->setValueNotifyingHost(normalizedValue);
        }
        
        lfo.setRate(static_cast<float>(syncedFreq));
    } else {
        // In manual mode, just use the rate parameter directly
        lfo.setRate(rateParam->load());
        
        // Store the current manual rate in case we switch to sync mode
        lfo.storeManualRate(rateParam->load());
    }

    // Pre-calculate LFO values with high quality
    if (isPlaying && (hasSignal || lfo.isWaitingForReset()))
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
        
        if (isPlaying && (hasSignal || lfo.isWaitingForReset()))
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
void QuackerVSTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void QuackerVSTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QuackerVSTAudioProcessor();
}

double QuackerVSTAudioProcessor::getCurrentBPM() const
{
    return currentBPM;
}
