#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ClubMasterAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    ClubMasterAudioProcessorEditor(ClubMasterAudioProcessor&);
    ~ClubMasterAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
private:
    ClubMasterAudioProcessor& audioProcessor;
    juce::Slider inputKnob{ juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::ComboBox modeBox;
    juce::Slider ceilingSlider, releaseSlider, linkSlider;
    juce::ToggleButton hqButton{ "HQ" };
    juce::Label lufsLabel, peakLabel, grLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a1, a2, a3, a4;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aMode;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aHq;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClubMasterAudioProcessorEditor)
};
