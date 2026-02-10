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
    driveSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    driveSlider.setRange (0.0, 127.0, 1.0);
    driveSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    driveSlider.setPopupDisplayEnabled (true, false, this);
    driveSlider.setTextValueSuffix ("Drive");
    driveSlider.setValue (1.0);
    
    distTypeBox.addItem("Tanh", 1);
    distTypeBox.addItem("Soft Clip", 2);
    distTypeBox.addItem("Hard Clip", 3);
    
    // add the components to the editor
    addAndMakeVisible(&driveSlider);
    addAndMakeVisible(&distTypeBox);
    
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "drive",
        driveSlider);

    distAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts,
        "distType",
        distTypeBox);

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
    g.drawFittedText ("Distortion Plugin", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    
}

void Distortion_pluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    driveSlider.setBounds(40, 30, 20, getHeight() - 60);
    
    distTypeBox.setBounds(200, 50, 20, getHeight() - 60);
    
}