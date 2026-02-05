/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Distortion_pluginAudioProcessorEditor::Distortion_pluginAudioProcessorEditor (Distortion_pluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 600);
    Volume.setSliderStyle (juce::Slider::LinearBarVertical);
    Volume.setRange (0.0, 127.0, 1.0);
    Volume.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    Volume.setPopupDisplayEnabled (true, false, this);
    Volume.setTextValueSuffix (" Volume");
    Volume.setValue (1.0);
    
    // add the slider to the editor
    addAndMakeVisible (&Volume);
    
}

Distortion_pluginAudioProcessorEditor::~Distortion_pluginAudioProcessorEditor()
{
}

//==============================================================================
void Distortion_pluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::white);

    g.setColour (juce::Colours::black);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Volume", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    
}

void Distortion_pluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    Volume.setBounds (40, 30, 20, getHeight() - 60);
    
    
    
}