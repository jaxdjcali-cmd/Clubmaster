#include "PluginProcessor.h"
#include "PluginEditor.h"

ClubMasterAudioProcessor::ClubMasterAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParams())
{
}

ClubMasterAudioProcessor::~ClubMasterAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ClubMasterAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("input", "Input Gain", -12.f, 12.f, 0.f));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("mode", "Master Mode",
        juce::StringArray{ "Squash Loud (-7 LUFS)", "Club Ready (-9 LUFS)", "Streaming (-14 LUFS)" }, 1));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("ceiling", "True-Peak Ceiling", -3.f, -0.1f, -1.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 20.f, 500.f, 120.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("link", "Stereo Link", 0.f, 1.f, 0.7f));
    p.push_back(std::make_unique<juce::AudioParameterBool>("quality", "HQ", false));
    return { p.begin(), p.end() };
}

void ClubMasterAudioProcessor::prepareToPlay(double sr, int)
{
    sampleRate = sr;
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, 38.0f);
    for (auto& f : kFilter) f.coefficients = coeffs;
    envelope = 0.f; autoGain = 1.f; smoothedGain = 1.f;
    currentLUFS = -20.f; truePeakDb = -60.f; grDb = 0.f;
}

void ClubMasterAudioProcessor::releaseResources() {}

void ClubMasterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float inputGainDb = *apvts.getRawParameterValue("input");
    int mode = (int)*apvts.getRawParameterValue("mode");
    float targetLUFS = mode == 0 ? -7.f : (mode == 2 ? -14.f : -9.f);
    float ceilingDb = *apvts.getRawParameterValue("ceiling");
    float releaseMs = *apvts.getRawParameterValue("release");

    float inGain = juce::Decibels::decibelsToGain(inputGainDb);
    const int N = buffer.getNumSamples(), Ch = buffer.getNumChannels();

    // 1) Medir LUFS aprox
    double sum = 0; int cnt = 0;
    for (int s = 0; s < N; s++)
        for (int c = 0; c < Ch; c++)
        {
            float v = kFilter[c].processSample(buffer.getSample(c, s) * inGain);
            sum += v * v; cnt++;
        }
    float rms = cnt ? std::sqrt(sum / cnt) : 0.f;
    float meas = rms > 1e-9f ? 20 * std::log10(rms) - 0.691f : -70.f;
    currentLUFS = 0.9f * currentLUFS + 0.1f * meas;

    // 2) Auto-gain suave hacia el target
    float desired = juce::jlimit(0.25f, 4.0f, juce::Decibels::decibelsToGain(targetLUFS - meas));
    autoGain = 0.98f * autoGain + 0.02f * desired;
    float totalGain = inGain * autoGain;
    float ceiling = juce::Decibels::decibelsToGain(ceilingDb);
    float releaseCoeff = std::exp(-1.0f / (float)(sampleRate * releaseMs / 1000.f));
    float peak = 0.f;

    // 3) Limiter
    for (int s = 0; s < N; s++)
    {
        float linkedPeak = 0.f;
        for (int c = 0; c < Ch; c++)
            linkedPeak = std::max(linkedPeak, std::abs(buffer.getSample(c, s) * totalGain));
        envelope = std::max(linkedPeak, envelope * releaseCoeff);
        if (envelope > ceiling) smoothedGain = std::min(smoothedGain, ceiling / envelope);
        else smoothedGain += (1.0f - smoothedGain) * (1 - releaseCoeff) * 0.5f;
        for (int c = 0; c < Ch; c++)
        {
            float y = buffer.getSample(c, s) * totalGain * smoothedGain;
            peak = std::max(peak, std::abs(y));
            buffer.setSample(c, s, juce::jlimit(-1.0f, 1.0f, y));
        }
    }
    truePeakDb = 20 * std::log10(std::max(peak, 1e-9f));
    grDb = 20 * std::log10(std::max(smoothedGain, 1e-4f));
}

juce::AudioProcessorEditor* ClubMasterAudioProcessor::createEditor()
{
    return new ClubMasterAudioProcessorEditor(*this);
}

void ClubMasterAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ClubMasterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClubMasterAudioProcessor();
}
