/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Distortion_pluginAudioProcessor::Distortion_pluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), 
                       apvts(*this, nullptr, "PARAMETERS", createParameters())
#endif
{
}

Distortion_pluginAudioProcessor::~Distortion_pluginAudioProcessor()
{
}

//==============================================================================
const juce::String Distortion_pluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Distortion_pluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Distortion_pluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Distortion_pluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Distortion_pluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Distortion_pluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Distortion_pluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Distortion_pluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Distortion_pluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void Distortion_pluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Distortion_pluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Distortion_pluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Distortion_pluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

juce::AudioProcessorValueTreeState::ParameterLayout
Distortion_pluginAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive",
        "Drive",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f),
        6.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix",
        "Mix",
        0.0f,
        1.0f,
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "distType",
        "Distortion Type",
        juce::StringArray{"Tanh", "Soft Clip", "Hard Clip", "Cubic"},
        0));

    return { params.begin(), params.end() };
}

float Distortion_pluginAudioProcessor::processDistortion(float x, DistortionType type)
{
    switch (type)
    {
    case DistortionType::tanh:
        return std::tanh(x);

    case DistortionType::softClip:
        return x / (1.0f + std::abs(x));

    case DistortionType::hardClip:
        return juce::jlimit(-1.0f, 1.0f, x);
        
    case DistortionType::cubic:
        return x * x * x;

    default:
        return x;
    }
}

void Distortion_pluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numChannels = buffer.getNumChannels();
    auto numSamples = buffer.getNumSamples();
    
    float driveDb = *apvts.getRawParameterValue("drive");
    float driveGain = juce::Decibels::decibelsToGain(driveDb);
    
    float mix = *apvts.getRawParameterValue("mix");

    auto distType = static_cast<DistortionType>((int)*apvts.getRawParameterValue("distType"));



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
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        for (int i = 0; i < numSamples; ++i) {
            
            auto drySignal = channelData[i];
            auto wetSignal = drySignal;
            
            wetSignal *= driveGain;
            wetSignal = processDistortion(wetSignal, distType);
            
            float outputGain = 1.0f / driveGain;
            wetSignal *= outputGain;
            
            // Mix dry and wet signals based on mix parameter (0 = dry, 1 = wet)
            channelData[i] = drySignal * (1.0f - mix) + wetSignal * mix;
        }
    }
}

//==============================================================================
bool Distortion_pluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Distortion_pluginAudioProcessor::createEditor()
{
    return new Distortion_pluginAudioProcessorEditor (*this);
}

//==============================================================================
void Distortion_pluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);

}

void Distortion_pluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Distortion_pluginAudioProcessor();
}
