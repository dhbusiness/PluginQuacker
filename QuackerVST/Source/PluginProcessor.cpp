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
lfoRateParam(new juce::AudioParameterFloat("lfoRate", "LFO Rate", 0.01f, 2.0f, 1.0f)),      //Adding LFO params to constructor
lfoDepthParam(new juce::AudioParameterFloat("lfoDepth", "LFO Depth", 0.0f, 1.f, 0.5f)),      //Adding LFO params to constructor
lfoWaveformParam(new juce::AudioParameterChoice(
    "lfoWaveform", "LFO Waveform",
    juce::StringArray{ "Sine", "Square", "Triangle" }, 0 )),// Default: Sine - Adding LFO waveform selection to constructor
lfoSyncParam(new juce::AudioParameterBool("lfoSync", "LFO Sync", false)),
lfoNoteDivisionParam(new juce::AudioParameterChoice(
    "lfoNoteDivision", "LFO Note Division",
    juce::StringArray{ "Whole", "Half", "Quarter", "Eighth", "Sixteenth" }, 2)), // Default: Quarter note
lfoPhaseOffsetParam(new juce::AudioParameterFloat("lfoPhaseOffset", "LFO Phase Offset", -180.0f, 180.0f, 0.0f)) // Range: -180° to 180°, default 0°
{
    addParameter(lfoRateParam);         //Init LFO params
    addParameter(lfoDepthParam);        //Init LFO params
    addParameter(lfoWaveformParam);
    addParameter(lfoSyncParam);
    addParameter(lfoNoteDivisionParam);
    addParameter(lfoPhaseOffsetParam);
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
    
    
    // Map the parameter value to the corresponding waveform
    TremoloLFO::Waveform selectedWaveform = static_cast<TremoloLFO::Waveform>(lfoWaveformParam->getIndex());
    lfo.setWaveform(selectedWaveform);
    
    //Updates BPM in every processing block
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo))
        {
            currentBPM = posInfo.bpm;
        }
    }
    // LFO Beat sync processing and LFO param setting
    if (lfoSyncParam->get())
    {
        // Note division to rate mapping
        static const std::map<int, double> divisionToMultiplier = {
            { 0, 4.0 },  // Whole note
            { 1, 2.0 },  // Half note
            { 2, 1.0 },  // Quarter note
            { 3, 0.5 },  // Eighth note
            { 4, 0.25 }  // Sixteenth note
        };

        double multiplier = divisionToMultiplier.at(lfoNoteDivisionParam->getIndex());
        lfo.setRate(currentBPM / (60.0 * multiplier));
    }
    else
    {
        lfo.setRate(lfoRateParam->get());
    }

    lfo.setDepth(lfoDepthParam->get());
    lfo.setPhaseOffset(lfoPhaseOffsetParam->get()); // Apply phase offset
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
}

void QuackerVSTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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
