/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"

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

QuackerVSTAudioProcessor::~QuackerVSTAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout QuackerVSTAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoRate", "LFO Rate", 0.01f, 2.0f, 1.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoDepth", "LFO Depth", 0.0f, 1.0f, 0.5f
    ));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "lfoWaveform",
        "LFO Waveform",
        juce::StringArray{
            "Sine", "Square", "Triangle", "Sawtooth Up",
            "Sawtooth Down", "Soft Square", "Fender Style", "Wurlitzer Style"
        },
        0
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "lfoSync", "LFO Sync", false
    ));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "lfoNoteDivision",
        "LFO Note Division",
        juce::StringArray{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" },
        2
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoPhaseOffset", "LFO Phase Offset", -180.0f, 180.0f, 0.0f
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix", 0.0f, 1.0f, 1.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "bypass", "Bypass", false
    ));

    return { params.begin(), params.end() };
}

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
    return 0.1; // Return actual tail time based on tremolo settings
}

int QuackerVSTAudioProcessor::getNumPrograms()
{
    return 1;
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

void QuackerVSTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize DSP spec
    dspSpec.maximumBlockSize = samplesPerBlock;
    dspSpec.sampleRate = sampleRate;
    dspSpec.numChannels = getTotalNumOutputChannels();
    
    tempBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    
    // Initialize anti-aliasing filter
    antiAliasingFilter.prepare(dspSpec);
    auto& filter = antiAliasingFilter.state;
    
    // Adjust filter frequency based on buffer size
    float cutoffFreq = sampleRate * 0.45f;  // Nyquist adjustment
    if (samplesPerBlock > 128)
    {
        // For larger buffer sizes, be more conservative with the cutoff
        cutoffFreq = std::min(cutoffFreq, 18000.0f);
    }
    *filter = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffFreq);
    
    // Initialize LFO with adjusted parameters for larger buffer sizes
    lfo.setSampleRate(sampleRate);
    
    // Adjust smoothing times based on buffer size
    float smoothingTime = juce::jmap((float)samplesPerBlock,
                                   32.0f, 2048.0f,  // Input range
                                   0.1f, 0.25f);    // Output range (100ms to 250ms)
    
    // Reserve memory for processing
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    // Clear ring buffer
    std::fill(ringBuffer.begin(), ringBuffer.end(), 0.0f);
    fifo.reset();
}

void QuackerVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void QuackerVSTAudioProcessor::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer == nullptr) return;
    
    auto* buffer = bufferToFill.buffer;
    juce::dsp::AudioBlock<float> block(*buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Apply anti-aliasing if needed
    if (lfo.isOversamplingEnabled())
    {
        antiAliasingFilter.process(context);
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool QuackerVSTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
}
#endif

void QuackerVSTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numSamples = buffer.getNumSamples();
    
    // Cache playhead info at block rate
    bool isPlaying = false;
    static juce::AudioPlayHead::CurrentPositionInfo posInfo;
    if (auto* playHead = getPlayHead())
    {
        if (playHead->getCurrentPosition(posInfo))
        {
            isPlaying = posInfo.isPlaying;
            currentBPM = posInfo.bpm;
            currentlyPlaying = isPlaying;
        }
    }

    // Optimized signal detection using SIMD
    bool hasSignal = false;
    for (int channel = 0; channel < buffer.getNumChannels() && !hasSignal; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        if (std::abs(juce::FloatVectorOperations::findMaximum(channelData, numSamples)) > 0.0001f)
        {
            hasSignal = true;
            break;
        }
    }

    // Cache parameters at block rate
    const bool isBypassed = apvts.getRawParameterValue("bypass")->load();
    if (isBypassed)
    {
        audioInputDetected = hasSignal;
        currentlyPlaying = isPlaying;
        return;
    }

    const auto currentWaveform = static_cast<TremoloLFO::Waveform>(
        static_cast<int>(apvts.getRawParameterValue("lfoWaveform")->load()));
    const float currentDepth = apvts.getRawParameterValue("lfoDepth")->load();
    const float currentPhaseOffset = apvts.getRawParameterValue("lfoPhaseOffset")->load();
    const bool syncEnabled = apvts.getRawParameterValue("lfoSync")->load() > 0.5f;
    const float currentRate = apvts.getRawParameterValue("lfoRate")->load();
    const float currentMix = apvts.getRawParameterValue("mix")->load();

    // Update LFO state
    audioInputDetected = hasSignal;
    bool isActive = isPlaying && hasSignal;
    lfo.updateActiveState(isActive, isPlaying);

    // Configure LFO
    lfo.setWaveform(currentWaveform);
    lfo.setDepth(currentDepth);
    lfo.setPhaseOffset(currentPhaseOffset);

    if (syncEnabled)
    {
        const double divisions[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
        double division = divisions[static_cast<int>(apvts.getRawParameterValue("lfoNoteDivision")->load())];
        lfo.setSyncMode(true, division);
        if (isPlaying)
        {
            lfo.setBeatPosition(posInfo.ppqPosition);
        }
    }
    else
    {
        lfo.setSyncMode(false);
        lfo.setRate(currentRate);
    }

    // Pre-calculate mix gains using equal power crossfade
    const float wetGain = std::sin(currentMix * juce::MathConstants<float>::halfPi);
    const float dryGain = std::cos(currentMix * juce::MathConstants<float>::halfPi);

    // Process audio with optimized buffer operations
    if (isPlaying && (hasSignal || lfo.isWaitingForReset()))
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            
            // Store dry signal
            if (currentMix < 1.0f)
            {
                tempBuffer.copyFrom(channel, 0, channelData, numSamples);
            }

            // Apply modulation
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float lfoValue = lfo.getNextSample();
                float smoothedLFO = 0.5f + (lfoValue - 0.5f) * 0.707f;
                channelData[sample] *= smoothedLFO;
            }

            // Apply soft saturation
            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] = std::tanh(channelData[sample]);
            }

            // Mix wet and dry signals
            if (currentMix < 1.0f)
            {
                juce::FloatVectorOperations::multiply(channelData, wetGain, numSamples);
                juce::FloatVectorOperations::addWithMultiply(channelData, tempBuffer.getReadPointer(channel),
                                                           dryGain, numSamples);
            }
        }

        // Apply gentle limiting
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            juce::FloatVectorOperations::multiply(channelData, 0.99f, numSamples);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                channelData[sample] = std::tanh(channelData[sample]);
            }
        }
    }

    // Apply anti-aliasing if needed
    if (lfo.isOversamplingEnabled())
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        antiAliasingFilter.process(context);
    }
}

bool QuackerVSTAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* QuackerVSTAudioProcessor::createEditor()
{
    return new QuackerVSTAudioProcessorEditor (*this);
}

void QuackerVSTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("version", 1, nullptr);
    state.setProperty("lastSavedDate", juce::Time::getCurrentTime().toString(true, true), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void QuackerVSTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

double QuackerVSTAudioProcessor::getCurrentBPM() const
{
    return currentBPM;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QuackerVSTAudioProcessor();
}
