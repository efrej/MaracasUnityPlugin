/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MaracasPluginAudioProcessor::MaracasPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(excite = new AudioParameterBool("excite", // parameter ID
        "Excite", // parameter name
        false   // default value
        )); // default value
    
}

MaracasPluginAudioProcessor::~MaracasPluginAudioProcessor()
{
}

//==============================================================================
const juce::String MaracasPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MaracasPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MaracasPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MaracasPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MaracasPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MaracasPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MaracasPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MaracasPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MaracasPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void MaracasPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MaracasPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    // initialize gourd resonance frequencies
    Fs = sampleRate;

    gain = static_cast<float>(log(numBeans)) / static_cast<float>(log(4)) / static_cast<float>(2000 * numBeans);

    coef1 = -shellReso * 2.0f * cos(shellFreq * double_Pi * 2.f / sampleRate);
    coef2 = shellReso * shellReso;
}

void MaracasPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MaracasPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MaracasPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.



        // ..do something to the data...
        for (int n = 0; n < buffer.getNumSamples(); n++)
        {
            t++;
            auto outL = buffer.getWritePointer(0);
            auto outR = buffer.getWritePointer(1);

            if (temp < M_PI * 2)
            {
                temp += (M_PI * 2 / Fs / shakeTime);
                shakeEnergy += (1 - cos(temp));
            }

            if (*excite) //change to "when exited"
            {
                temp = 0;
                shakeTime = 4.f * r.nextFloat() / 100.f + 0.03f; //random value in between 0.04 and 0.08
                *excite = false;
            }

            shakeEnergy *= systemDecay;

            if (r.nextInt(Range<int>(1, 1048)) < numBeans)
            {
                soundLevel += gain * shakeEnergy;
            }

            auto input = soundLevel * 2.f * r.nextFloat() - 1.f;
            soundLevel *= soundDecay;
            input -= x * coef1;
            input -= x1 * coef2;
            x1 = x;
            x = input;

            auto data = x - x1;
            
            data = limit(data);

            outL[n] = data;
            outR[n] = data;

        }
}

//==============================================================================
bool MaracasPluginAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MaracasPluginAudioProcessor::createEditor()
{
    return new MaracasPluginAudioProcessorEditor (*this);
}

//==============================================================================
void MaracasPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MaracasPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaracasPluginAudioProcessor();
}

float MaracasPluginAudioProcessor::limit(float n)
{
    if (n < -1.f) return -1.f;
    else if (n > 1.f) return 1.f;
    else return n;
}
