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
    setSize (500, 300);
    
    // Drive Slider
    driveSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    driveSlider.setRange (0.0, 127.0, 1.0);
    driveSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    driveSlider.setPopupDisplayEnabled (true, false, this);
    driveSlider.setTextValueSuffix ("Drive");
    driveSlider.setValue (1.0);
    
    // Mix Slider
    mixSlider.setSliderStyle (juce::Slider::LinearBarVertical);
    mixSlider.setRange (0.0, 1.0, 0.01f);
    mixSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    mixSlider.setPopupDisplayEnabled (true, false, this);
    mixSlider.setTextValueSuffix ("Mix");
    mixSlider.setValue (1.0);
    
    // Distortion Type Box
    distTypeBox.addItem("Tanh", 1);
    distTypeBox.addItem("Soft Clip", 2);
    distTypeBox.addItem("Hard Clip", 3);
    distTypeBox.addItem("Cubic", 4);
    
    // Filter State Box
    filterStateBox.addItem("Off", 1);
    filterStateBox.addItem("Pre-Distortion", 2);
    filterStateBox.addItem("Post-Distortion", 3);
    filterStateBox.setSelectedItemIndex(0);
    
    // Filter Type Box
    filterTypeBox.addItem("Lowpass", 1);
    filterTypeBox.addItem("Highpass", 2);
    filterTypeBox.setSelectedItemIndex(0);
    
    // Filter Frequency Slider
    filterFreqSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    filterFreqSlider.setRange (20.0, 20000.0, 1.0);
    filterFreqSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 0);
    filterFreqSlider.setPopupDisplayEnabled (true, false, this);
    filterFreqSlider.setTextValueSuffix (" Hz");
    filterFreqSlider.setValue (1000.0);
    filterFreqSlider.setSkewFactorFromMidPoint(1000.0);
    
    // Filter Resonance Slider
    filterResSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    filterResSlider.setRange (0.1, 10.0, 0.01f);
    filterResSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 0);
    filterResSlider.setPopupDisplayEnabled (true, false, this);
    filterResSlider.setTextValueSuffix (" Q");
    filterResSlider.setValue (1.0);
    
    // Add components
    addAndMakeVisible(&driveSlider);
    addAndMakeVisible(&mixSlider);
    addAndMakeVisible(&distTypeBox);
    addAndMakeVisible(&filterStateBox);
    addAndMakeVisible(&filterTypeBox);
    addAndMakeVisible(&filterFreqSlider);
    addAndMakeVisible(&filterResSlider);
    
    // Create attachments
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "drive",
        driveSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "mix",
        mixSlider);

    distAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts,
        "distType",
        distTypeBox);
    
    filterStateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts,
        "filterState",
        filterStateBox);
    
    filterTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts,
        "filterType",
        filterTypeBox);
    
    filterFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "filterFreq",
        filterFreqSlider);
    
    filterResAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts,
        "filterRes",
        filterResSlider);

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
    
    // Distortion section labels
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("Drive", 40, 30, 30, 20, juce::Justification::centred);
    g.drawText ("Mix", 80, 30, 30, 20, juce::Justification::centred);
    g.drawText ("Distortion Type", 30, 205, 120, 15, juce::Justification::centred);
    
    // Filter section labels
    g.drawText ("Filter State", 180, 30, 150, 15, juce::Justification::centred);
    g.drawText ("Filter Type", 180, 95, 150, 15, juce::Justification::centred);
    g.drawText ("Frequency", 180, 145, 150, 15, juce::Justification::centred);
    g.drawText ("Resonance (Q)", 180, 185, 150, 15, juce::Justification::centred);
}

void Distortion_pluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    // Distortion controls (left side)
    driveSlider.setBounds(40, 50, 20, 150);
    mixSlider.setBounds(80, 50, 20, 150);
    distTypeBox.setBounds(30, 220, 120, 30);
    
    // Filter controls (right side)
    filterStateBox.setBounds(180, 50, 150, 30);
    filterTypeBox.setBounds(180, 115, 150, 30);
    filterFreqSlider.setBounds(180, 165, 300, 30);
    filterResSlider.setBounds(180, 205, 300, 30);
}