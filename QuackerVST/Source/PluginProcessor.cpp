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
        juce::StringArray{ "Sine", "Square", "Triangle" },
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
        juce::StringArray{ "Whole", "Half", "Quarter", "Eighth", "Sixteenth" },
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    //init spec for dsp modules
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    lfo.setSampleRate(sampleRate); //SETS SAMPLE RATE FOR LFO TO USER SAMPLE RATE
    
}

void QuackerVSTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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

void QuackerVSTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    //auto gainValue = gainParameter->get(); //Added getting value from user input, this will be changed
    
    //Updates BPM in every processing block
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo))
        {
            currentBPM = posInfo.bpm;
        }
    }
    
    // Get parameters from APVTS
    auto* waveformParam = apvts.getRawParameterValue("lfoWaveform");
    auto* rateParam = apvts.getRawParameterValue("lfoRate");
    auto* depthParam = apvts.getRawParameterValue("lfoDepth");
    auto* syncParam = apvts.getRawParameterValue("lfoSync");
    auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
    auto* phaseOffsetParam = apvts.getRawParameterValue("lfoPhaseOffset");

    // Set LFO parameters
    lfo.setWaveform(static_cast<TremoloLFO::Waveform>(static_cast<int>(waveformParam->load())));
    
    if (syncParam->load() > 0.5f) // If sync is enabled
    {
        // Note division to rate mapping
        const double multipliers[] = { 0.25, 0.5, 1.0, 2.0, 4.0 }; // whole, half, quarter, eighth, sixteenth
        double multiplier = multipliers[static_cast<int>(divisionParam->load())];
        lfo.setRate(currentBPM / (60.0 * multiplier));
    }
    else
    {
        lfo.setRate(rateParam->load());
    }

    lfo.setDepth(depthParam->load());
    lfo.setPhaseOffset(phaseOffsetParam->load());
    //
    
    

    //CLEARS EMPTY CHANNELS
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
 
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        // ..do something to the data...
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float lfoValue = lfo.getNextSample();       //Pulling lfoData to be used
            channelData[sample] *= (1.0f - lfoValue);   //Applying LFO against the amplitude of the audio signal
        };
        
    };
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
