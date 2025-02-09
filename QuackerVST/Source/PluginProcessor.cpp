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
        2.0f,               // maximum value
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
    
    // Modulation LFO Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "modLfoRate",
        "Mod LFO Rate",
        0.01f,
        5.0f,
        0.5f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "modLfoDepth",
        "Mod LFO Depth",
        0.0f,
        1.0f,
        0.5f
    ));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "modLfoWaveform",
        "Mod LFO Waveform",
        juce::StringArray{"Sine", "Triangle", "Square", "Saw"},
        0  // default to Sine
    ));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "modTarget",
        "Modulation Target",
        juce::StringArray{"Rate", "Depth", "Phase"},
        0  // default to Rate
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "modEnable",
        "Modulation Enable",
        false  // default to off
    ));
    
    //Waveshape LFO
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "wsLfoRate",
        "Waveshape LFO Rate",
        0.01f,
        5.0f,
        0.5f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "wsLfoDepth",
        "Waveshape LFO Depth",
        0.0f,
        1.0f,
        0.5f
    ));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "wsLfoWaveform",
        "Waveshape LFO Waveform",
        juce::StringArray{"Sine", "Triangle", "Square", "Saw"},
        0  // default to Sine
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "wsEnable",
        "Waveshape Enable",
        false  // default to off
    ));

    //Interpolation control
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "interpolationType",
        "Interpolation Type",
        juce::StringArray{"Linear", "Cubic", "Hermite"},
        1  // default to Cubic
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

    // Allocate buffer with alignment for SIMD operations
    lfoValuesBuffer.allocate(samplesPerBlock + 4, true);

    // High-quality DC filter setup
    *dcFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate,
        5.0f,
        0.707f
    );
    dcFilter.prepare(currentSpecs);
    
    // Prepare both LFOs
    lfo.setSampleRate(sampleRate);
    modLFO.prepare(sampleRate);
    waveshapeLFO.prepare(sampleRate);
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
    
    // Get the interpolation type parameter
    auto* interpolationParam = apvts.getRawParameterValue("interpolationType");
    auto interpolationType = static_cast<TremoloLFO::InterpolationType>(
        static_cast<int>(interpolationParam->load()));
    
    // Get modulation values from both LFOs with interpolation
    float modValue = modLFO.getNextInterpolatedValue(interpolationType);
    float wsValue = waveshapeLFO.getNextInterpolatedValue(interpolationType);
    lastModValue = modValue;
    lastWaveshapeValue = wsValue;

    // Set interpolation type for main LFO
    lfo.setInterpolationType(interpolationType);

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
        audioInputDetected = hasSignal;
        currentlyPlaying = isPlaying;
        lfo.resetPhase();
        return;
    }

    // Update the member variables
    audioInputDetected = hasSignal;
    bool isActive = isPlaying && hasSignal;
    lfo.updateActiveState(isActive, isPlaying);

    // Get all LFO parameters
    auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
    auto* rateParam = apvts.getRawParameterValue("lfoRate");
    auto* depthParam = apvts.getRawParameterValue("lfoDepth");
    auto* phaseOffsetParam = apvts.getRawParameterValue("lfoPhaseOffset");
    auto* syncParam = apvts.getRawParameterValue("lfoSync");
    auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
    auto* mixParam = apvts.getRawParameterValue("mix");
    float mix = mixParam->load();

    // Update main LFO settings
    lfo.setWaveform(static_cast<TremoloLFO::Waveform>(static_cast<int>(waveformParam->load())));
    lfo.setDepth(depthParam->load());
    lfo.setPhaseOffset(phaseOffsetParam->load());

    // Get modulation enable states
    auto* modEnableParam = apvts.getRawParameterValue("modEnable");
    auto* wsEnableParam = apvts.getRawParameterValue("wsEnable");
    bool modEnabled = modEnableParam->load();
    bool wsEnabled = wsEnableParam->load();
    auto target = modLFO.getTarget();

    // Handle sync and timing
    if (syncParam->load() > 0.5f)
    {
        const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
        double division = divisions[static_cast<int>(divisionParam->load())];
        
        if (modEnabled && target == ModulationLFO::Target::Rate)
        {
            float modValue = modLFO.getNextInterpolatedValue(interpolationType);
            division = division * modLFO.applyModulation(1.0f, modValue);
        }
        
        lfo.setSyncMode(true, division);
        if (isPlaying)
        {
            lfo.setBeatPosition(posInfo.ppqPosition);
        }
    }
    else
    {
        lfo.setSyncMode(false);
        lfo.setRate(rateParam->load());
    }

    // Update both modulation LFOs
    auto* modRateParam = apvts.getRawParameterValue("modLfoRate");
    auto* modDepthParam = apvts.getRawParameterValue("modLfoDepth");
    auto* modWaveformParam = apvts.getRawParameterValue("modLfoWaveform");
    auto* modTargetParam = apvts.getRawParameterValue("modTarget");

    modLFO.setRate(modRateParam->load());
    modLFO.setDepth(modDepthParam->load());
    modLFO.setWaveform(static_cast<ModulationLFO::Waveform>(static_cast<int>(modWaveformParam->load())));
    modLFO.setTarget(static_cast<ModulationLFO::Target>(static_cast<int>(modTargetParam->load())));

    // Update waveshape LFO
    auto* wsRateParam = apvts.getRawParameterValue("wsLfoRate");
    auto* wsDepthParam = apvts.getRawParameterValue("wsLfoDepth");
    auto* wsWaveformParam = apvts.getRawParameterValue("wsLfoWaveform");

    waveshapeLFO.setRate(wsRateParam->load());
    waveshapeLFO.setDepth(wsDepthParam->load());
    waveshapeLFO.setWaveform(static_cast<ModulationLFO::Waveform>(static_cast<int>(wsWaveformParam->load())));

    // Get base parameter values outside the loop
    float baseRate = rateParam->load();
    float baseDepth = depthParam->load();
    float basePhaseOffset = phaseOffsetParam->load();

    // Process audio
    if (isPlaying && (hasSignal || lfo.isWaitingForReset()))
    {
        // Pre-calculate LFO values with modulation
        for (int i = 0; i < numSamples; ++i)
        {
            float modValue = modLFO.getNextInterpolatedValue(interpolationType);
            float wsValue = wsEnabled ?
                waveshapeLFO.getNextInterpolatedValue(interpolationType) :
                0.0f;
                
            lastModValue = modValue;
            lastWaveshapeValue = wsValue;

            // Calculate modulated values based on target
            float effectiveRate = baseRate;
            float effectiveDepth = baseDepth;
            float effectivePhaseOffset = basePhaseOffset;

            if (modEnabled)
            {
                switch (target)
                {
                    case ModulationLFO::Target::Rate:
                        effectiveRate = modLFO.applyModulation(baseRate, modValue);
                        break;
                    case ModulationLFO::Target::Depth:
                        effectiveDepth = modLFO.applyModulation(baseDepth, modValue);
                        break;
                    case ModulationLFO::Target::Phase:
                        effectivePhaseOffset = modLFO.applyModulation(basePhaseOffset, modValue);
                        break;
                }
            }

            // Update LFO parameters with modulated values
            lfo.setRate(effectiveRate);
            lfo.setDepth(effectiveDepth);
            lfo.setPhaseOffset(effectivePhaseOffset);
            
            // Get LFO sample with waveshaping and ensure it's in the correct range
            float lfoValue = lfo.getNextSample(wsValue);
            lfoValue = juce::jlimit(0.0f, 1.0f, lfoValue);
            lfoValuesBuffer[i] = lfoValue;
        }

        // Process audio channels
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            auto* dryData = buffer.getReadPointer(channel);

            juce::AudioBuffer<float> dryBuffer(1, numSamples);
            dryBuffer.copyFrom(0, 0, dryData, numSamples);

            juce::FloatVectorOperations::multiply(channelData, lfoValuesBuffer, numSamples);

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
    else
    {
        juce::FloatVectorOperations::fill(lfoValuesBuffer, 1.0f, numSamples);
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
