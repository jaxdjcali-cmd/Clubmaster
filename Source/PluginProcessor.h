#pragma once
#include <JuceHeader.h>

class ClubMasterAudioProcessor : public juce::AudioProcessor
{
public:
    ClubMasterAudioProcessor();
    ~ClubMasterAudioProcessor() override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Club Master"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    juce::AudioProcessorValueTreeState apvts;
    float getLUFS() const { return currentLUFS; }
    float getTruePeak() const { return truePeakDb; }
    float getGR() const { return grDb; }
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    double sampleRate = 48000.0;
    float currentLUFS = -20.f, truePeakDb = -60.f, grDb = 0.f;
    float envelope = 0.f, smoothedGain = 1.f;
    juce::AudioBuffer<float> lookaheadBuffer;
    int lookaheadPos = 0;
    juce::dsp::IIR::Filter<float> kFilter[2];
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClubMasterAudioProcessor)
};
