/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class Distortion_pluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Distortion_pluginAudioProcessor();
    ~Distortion_pluginAudioProcessor() override;

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
    
    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    //==============================================================================
    enum class DistortionType
    {
        tanh = 0,
        softClip,
        hardClip,
        cubic,
        arctan,
    };
    
    enum class FilterState
    {
        off = 0,
        preDistortion,
        postDistortion,
    };
    
    enum class FilterType
    {
        lowpass = 0,
        highpass,
    };
    
    float processDistortion(float x, DistortionType type);

    float audioVolume;
    
    // Updates filter coefficients when parameters change
    void updateFilterCoefficients(float freq, float res, FilterType type, FilterState state);

private:

    //==============================================================================
    // Filter DSP objects - one per channel
    std::array<juce::dsp::IIR::Filter<float>, 2> filters;
    
    // Cache filter parameter/state to avoid updating coefficients every block
    double currentSampleRate { 44100.0 };
    float prevFilterFreq { -1.0f };
    float prevFilterRes { -1.0f };
    FilterType prevFilterType { FilterType::lowpass };
    FilterState prevFilterState { FilterState::off };
    bool coefficientsInitialized { false };
    juce::dsp::ProcessSpec processSpec;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Distortion_pluginAudioProcessor)
};
