/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define _USE_MATH_DEFINES
#include <math.h>

//==============================================================================
/**
*/
class MaracasPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MaracasPluginAudioProcessor();
    ~MaracasPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    float limit (float n);
    
private:

    float Fs;
    float x = 0.f;
    float x1 = 0.f;

    float coef1;
    float coef2;

    float soundDecay = 0.95f;
    float systemDecay = 0.999f;
    float shellFreq = 7500;
    float shellReso = 0.98f;
    float reps = 2; // repetitions per second
    float temp = 10.f;
    float shakeEnergy = 0.f;
    float shakeTime = 0.05f; // 50 ms
    float soundLevel = 0.f;
    int numBeans = 64;
    float gain;
    
    int t = 0;
    
    Random r;
    
    AudioParameterBool* excite;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaracasPluginAudioProcessor)
};
