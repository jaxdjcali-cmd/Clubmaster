#include "PluginProcessor.h"
#include "PluginEditor.h"

ClubMasterAudioProcessorEditor::ClubMasterAudioProcessorEditor(ClubMasterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(620, 380);
    startTimerHz(15);
    addAndMakeVisible(inputKnob);
    inputKnob.setRange(-12, 12, 0.1);
    inputKnob.setTextValueSuffix(" dB");
    addAndMakeVisible(modeBox);
    modeBox.addItemList({ "Squash Loud -7 LUFS", "Club Ready -9 LUFS (recomendado)", "Streaming -14 LUFS" }, 1);
    addAndMakeVisible(ceilingSlider);
    ceilingSlider.setRange(-3, -0.1, 0.1);
    ceilingSlider.setTextValueSuffix(" dBTP");
    addAndMakeVisible(releaseSlider);
    releaseSlider.setRange(20, 500, 1);
    releaseSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(linkSlider);
    linkSlider.setRange(0, 1, 0.01);
    addAndMakeVisible(hqButton);
    addAndMakeVisible(lufsLabel);
    addAndMakeVisible(peakLabel);
    addAndMakeVisible(grLabel);
    a1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "input", inputKnob);
    aMode = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "mode", modeBox);
    a2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "ceiling", ceilingSlider);
    a3 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "release", releaseSlider);
    a4 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "link", linkSlider);
    aHq = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "quality", hqButton);
}
ClubMasterAudioProcessorEditor::~ClubMasterAudioProcessorEditor() { stopTimer(); }
void ClubMasterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111111));
    g.setColour(juce::Colours::white);
    g.setFont(22.f);
    g.drawText("CLUB MASTER - Mastering para DJs", 10, 10, 600, 30, juce::Justification::left);
    g.setFont(13.f);
    g.setColour(juce::Colours::lightgrey);
    g.drawText("1. Pon el Input en la marca  2. Elige tu master", 10, 38, 600, 20, juce::Justification::left);
}
void ClubMasterAudioProcessorEditor::resized()
{
    inputKnob.setBounds(30, 80, 150, 150);
    modeBox.setBounds(220, 80, 360, 30);
    ceilingSlider.setBounds(220, 130, 360, 30);
    releaseSlider.setBounds(220, 175, 360, 30);
    linkSlider.setBounds(220, 220, 360, 30);
    hqButton.setBounds(220, 265, 80, 25);
    lufsLabel.setBounds(30, 250, 160, 25);
    peakLabel.setBounds(30, 280, 160, 25);
    grLabel.setBounds(30, 310, 160, 25);
}
void ClubMasterAudioProcessorEditor::timerCallback()
{
    lufsLabel.setText("LUFS: " + juce::String(audioProcessor.getLUFS(), 1), juce::dontSendNotification);
    peakLabel.setText("True Peak: " + juce::String(audioProcessor.getTruePeak(), 1) + " dB", juce::dontSendNotification);
    grLabel.setText("Gain Reduction: " + juce::String(audioProcessor.getGR(), 1) + " dB", juce::dontSendNotification);
}
