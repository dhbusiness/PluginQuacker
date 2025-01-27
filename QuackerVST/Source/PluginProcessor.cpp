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
gainParameter(new juce::AudioParameterFloat("gain", "Gain", 0.0f, 2.0f, 1.0f)),
//ADSR Params added
attackParam(new juce::AudioParameterFloat("attack", "Attack", 0.01f, 2.0f, 0.1f)),
decayParam(new juce::AudioParameterFloat("decay", "Decay", 0.01f, 2.0f, 0.1f)),
sustainParam(new juce::AudioParameterFloat("sustain", "Sustain", 0.0f, 1.0f, 0.0f)),
releaseParam(new juce::AudioParameterFloat("release", "Release", 0.01f, 2.0f, 0.1f)),
duckingEnabled(new juce::AudioParameterBool("duckingEnabled", "Ducking Enabled", true)),
noteIntervalParam(new juce::AudioParameterChoice("noteInterval", "Note Interval",
                                                 { "Whole, Half, Quarter", "Eighth", "Sixteenth", "Thirty-Two"}, 2)) //Defaults to quarter because of the number at the end
{ //Added gain parameter
    addParameter(gainParameter);
    //ADSR params init
    addParameter(attackParam);
    addParameter(decayParam);
    addParameter(sustainParam);
    addParameter(releaseParam);
    addParameter(duckingEnabled);
    addParameter(noteIntervalParam);
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
    
    // Calculate samples per quarter note based on BPM
    samplesPerQuarterNote = (60.0 / currentBPM) * getSampleRate();

    // Adjust the interval multiplier for note durations relative to a quarter note
    switch (noteIntervalParam->getIndex()) // Selects math for note interval
    {
        case 0: intervalMultiplier = 4.0; break;   // Whole note (4 quarter notes)
        case 1: intervalMultiplier = 2.0; break;   // Half note (2 quarter notes)
        case 2: intervalMultiplier = 1.0; break;   // Quarter note (1 quarter note)
        case 3: intervalMultiplier = 0.5; break;   // Eighth note (0.5 quarter note)
        case 4: intervalMultiplier = 0.25; break;  // Sixteenth note (0.25 quarter note)
        case 5: intervalMultiplier = 0.125; break; // Thirty-second note (0.125 quarter note)
    }

    // Calculate selected samples per note interval
    selectedSamplePerNoteInterval = samplesPerQuarterNote * intervalMultiplier;
    
    /*
    switch (noteIntervalParam->getIndex()) //Selects math for note interval
        {
            case 0: intervalMultiplier = 4.0; break; // Whole note
            case 1: intervalMultiplier = 2.0; break; // Half note
            case 2: intervalMultiplier = 1.0; break; // Quarter note
            case 3: intervalMultiplier = 0.5; break; // Eighth note
            case 4: intervalMultiplier = 0.25; break; // Sixteenth note
            case 5: intervalMultiplier = 0.125; break; // Thirty-two note
        }
    
    selectedSamplePerNoteInterval = samplesPerQuarterNote * intervalMultiplier; //Does the math for selected note interval
    */
    
    
    //Update ADSR params
    /*
    adsrParams.attack = attackParam->get();
    adsrParams.decay = decayParam->get();
    adsrParams.sustain = sustainParam->get();
    adsrParams.release = releaseParam->get();
    adsr.setParameters(adsrParams);
     */
    

    DBG("BPM: " << currentBPM);
    DBG("Quarter Note Samples: " << samplesPerQuarterNote);
    DBG("Interval Multiplier: " << intervalMultiplier);
    DBG("Selected Interval Samples: " << selectedSamplePerNoteInterval);
    
    // Update ADSR parameters
    double totalADSRDuration = attackParam->get() + decayParam->get() + releaseParam->get();
    if (totalADSRDuration > (selectedSamplePerNoteInterval / getSampleRate()))
    {
        double scaleFactor = (selectedSamplePerNoteInterval / getSampleRate()) / totalADSRDuration;
        adsrParams.attack = attackParam->get() * scaleFactor;
        adsrParams.decay = decayParam->get() * scaleFactor;
        adsrParams.release = releaseParam->get() * scaleFactor;
    }
    else
    {
        adsrParams.attack = attackParam->get();
        adsrParams.decay = decayParam->get();
        adsrParams.release = releaseParam->get();
    }
    adsrParams.sustain = sustainParam->get();
    adsr.setParameters(adsrParams);
    
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
   // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
   //     buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if (duckingEnabled->get())
            {
                // Trigger ADSR at the start of the note interval
                if (sampleCount >= selectedSamplePerNoteInterval)
                {
                    adsr.noteOn(); // Start the envelope
                    sampleCount = 0.0; // Reset interval counter
                }

                // Release ADSR halfway through interval
                if (sampleCount >= selectedSamplePerNoteInterval / 2)
                {
                    adsr.noteOff();
                }

                // Apply ADSR envelope
                float envelopeValue = adsr.getNextSample();
                channelData[sample] *= (1.0f - envelopeValue);
            }
        }
    }
    
    // Increment sample count
    sampleCount += buffer.getNumSamples();
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
