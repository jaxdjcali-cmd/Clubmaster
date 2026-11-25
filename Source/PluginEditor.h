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
    juce::Slider inputKnob{ juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };
    juce::TextButton modeSquash{ "SQUASH LOUD  -7" };
    juce::TextButton modeClub{ "CLUB READY  -9" };
    juce::TextButton modeStream{ "STREAMING  -14" };
    juce::Slider ceilingSlider{ juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::Slider releaseSlider{ juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::Slider linkSlider{ juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::ToggleButton hqButton{ "HQ" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a1, a2, a3, a4;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aHq;
    float meterLUFS = -20.f, meterPeak = -20.f, meterGR = 0.f;
    float smoothLUFS = -20.f, smoothPeak = -20.f, smoothGR = 0.f;
    void updateModeButtons();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClubMasterAudioProcessorEditor)
};
