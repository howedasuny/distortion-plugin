/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Distortion_pluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Distortion_pluginAudioProcessorEditor (Distortion_pluginAudioProcessor&);
    ~Distortion_pluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Distortion_pluginAudioProcessor& audioProcessor;
    
    // Distortion controls
    juce::Slider driveSlider;
    juce::Slider mixSlider;
    juce::ComboBox distTypeBox;
    
    // Filter controls
    juce::ComboBox filterStateBox;
    juce::ComboBox filterTypeBox;
    juce::Slider filterFreqSlider;
    juce::Slider filterResSlider;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterStateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Distortion_pluginAudioProcessorEditor)
};
