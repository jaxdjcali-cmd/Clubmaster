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
    juce::TextButton modeSquash{ "SQUASH LOUD -7" };
    juce::TextButton modeClub{ "CLUB READY -9" };
    juce::TextButton modeStream{ "STREAMING -14" };
    juce::Slider ceilingSlider, releaseSlider, linkSlider;
    juce::ToggleButton hqButton{ "HQ" };
    juce::Label lufsLabel, peakLabel, grLabel, titleLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a1, a2, a3, a4;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aHq;
    float meterLUFS = -20.f, meterPeak = -20.f, meterGR = 0.f;
    void updateModeButtons();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClubMasterAudioProcessorEditor)
};
