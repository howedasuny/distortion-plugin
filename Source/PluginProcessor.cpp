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
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;  // Prepare each filter individually
    
    processSpec = spec;
    for (auto& f : filters)
        f.prepare(spec);
    
    currentSampleRate = sampleRate;
    // initialize coefficients with current APVTS values
    float filterFreq = *apvts.getRawParameterValue("filterFreq");
    float filterRes = *apvts.getRawParameterValue("filterRes");
    auto filterType = static_cast<FilterType>((int)*apvts.getRawParameterValue("filterType"));
    auto filterState = static_cast<FilterState>((int)*apvts.getRawParameterValue("filterState"));
    updateFilterCoefficients(filterFreq, filterRes, filterType, filterState);
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

    // Filter parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "filterState",
        "Filter State",
        juce::StringArray{"Off", "Pre-Distortion", "Post-Distortion"},
        0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "filterType",
        "Filter Type",
        juce::StringArray{"Lowpass", "Highpass"},
        0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterFreq",
        "Filter Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
        1000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterRes",
        "Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f),
        1.0f));

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

    case DistortionType::arctan:
        return (2.0f / juce::MathConstants<float>::pi) * std::atan(x);

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
    
    // Filter parameters
    auto filterState = static_cast<FilterState>((int)*apvts.getRawParameterValue("filterState"));
    auto filterType = static_cast<FilterType>((int)*apvts.getRawParameterValue("filterType"));
    float filterFreq = *apvts.getRawParameterValue("filterFreq");
    float filterRes = *apvts.getRawParameterValue("filterRes");
    
    // Update filter coefficients only when parameter/state changed
    bool stateChanged = (filterState != prevFilterState);
    
    if (!coefficientsInitialized
        || filterFreq != prevFilterFreq
        || filterRes != prevFilterRes
        || filterType != prevFilterType
        || stateChanged)
    {
        updateFilterCoefficients(filterFreq, filterRes, filterType, filterState);
        
        // Clear internal state when coefficients or mode changes
        if (stateChanged)
        {
            for (auto& f : filters)
                f.reset();
        }
    }

    // In case we have more outputs than inputs
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Make copies for dry/wet processing to avoid in-place conflicts
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    juce::AudioBuffer<float> wetBuffer;
    wetBuffer.makeCopyOf(buffer);

    // Apply pre-distortion filter to wetBuffer if enabled
    if (filterState == FilterState::preDistortion && coefficientsInitialized)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelPtr = wetBuffer.getWritePointer(channel);
            juce::dsp::AudioBlock<float> block(&channelPtr, 1, numSamples);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(block);
            filters[channel].process(ctx);
        }
    }

    // Process distortion on wetBuffer
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = wetBuffer.getWritePointer (channel);
        
        for (int i = 0; i < numSamples; ++i)
        {
            auto wetSignal = channelData[i];

            wetSignal *= driveGain;
            wetSignal = processDistortion(wetSignal, distType);

            float outputGain = 1.0f / driveGain;
            wetSignal *= outputGain;

            channelData[i] = wetSignal;
        }
    }

    // Apply post-distortion filter to wetBuffer if enabled
    if (filterState == FilterState::postDistortion && coefficientsInitialized)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelPtr = wetBuffer.getWritePointer(channel);
            juce::dsp::AudioBlock<float> block(&channelPtr, 1, numSamples);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(block);
            filters[channel].process(ctx);
        }
    }

    // Mix dry and wet into the output buffer
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* out = buffer.getWritePointer(channel);
        auto* dry = dryBuffer.getReadPointer(channel);
        auto* wet = wetBuffer.getReadPointer(channel);

        for (int i = 0; i < numSamples; ++i)
            out[i] = dry[i] * (1.0f - mix) + wet[i] * mix;
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

void Distortion_pluginAudioProcessor::updateFilterCoefficients(float freq, float res, FilterType type, FilterState state)
{
    // Only update coefficients when filter is active (not off)
    if (state == FilterState::off)
    {
        coefficientsInitialized = false;
        prevFilterFreq = freq;
        prevFilterRes = res;
        prevFilterType = type;
        prevFilterState = state;
        return;
    }

    // Clamp resonance/Q to a safe range to avoid unstable coefficients
    float q = juce::jlimit(0.1f, 10.0f, res);

    // Ensure cutoff is below Nyquist
    float nyquist = static_cast<float>(currentSampleRate * 0.5);
    float cutoff = juce::jlimit(20.0f, nyquist * 0.999f, freq);

    // Create the coefficient object and set it for all filters
    if (type == FilterType::lowpass)
    {
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, cutoff, q);
        for (auto& f : filters)
            f.coefficients = coeffs;
    }
    else
    {
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, cutoff, q);
        for (auto& f : filters)
            f.coefficients = coeffs;
    }

    // Reset internal filter state for all filters
    for (auto& f : filters)
        f.reset();

    coefficientsInitialized = true;
    prevFilterFreq = freq;
    prevFilterRes = res;
    prevFilterType = type;
    prevFilterState = state;
}
