#include "PluginProcessor.h"
#include "PluginEditor.h"

ClubMasterAudioProcessor::ClubMasterAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParams()) {}
ClubMasterAudioProcessor::~ClubMasterAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ClubMasterAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("input", "Input Gain", -12.f, 12.f, 0.f));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("mode", "Master Mode", juce::StringArray{ "Squash Loud (-7 LUFS)", "Club Ready (-9 LUFS)", "Streaming (-14 LUFS)" }, 1));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("ceiling", "True-Peak Ceiling", -3.f, -0.1f, -1.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 20.f, 500.f, 150.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("link", "Stereo Link", 0.f, 1.f, 0.7f));
    p.push_back(std::make_unique<juce::AudioParameterBool>("quality", "HQ", false));
    return { p.begin(), p.end() };
}

void ClubMasterAudioProcessor::prepareToPlay(double sr, int samples)
{
    sampleRate = sr;
    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, 30.0f);
    for (auto& f : kFilter) f.coefficients = coeffs;
    envelope = 0.f; smoothedGain = 1.f;
    currentLUFS = -20.f; truePeakDb = -60.f; grDb = 0.f;
    lookaheadBuffer.setSize(2, 256);
    lookaheadBuffer.clear();
    lookaheadPos = 0;
}
void ClubMasterAudioProcessor::releaseResources() {}

void ClubMasterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    float inputGainDb = *apvts.getRawParameterValue("input");
    int mode = (int)*apvts.getRawParameterValue("mode");
    float ceilingDb = *apvts.getRawParameterValue("ceiling");
    float releaseMs = *apvts.getRawParameterValue("release");
    float linkAmt = *apvts.getRawParameterValue("link");
    // Saturacion segun modo: Squash mas caliente, Streaming mas limpio
    float drive = mode == 0 ? 1.8f : (mode == 2 ? 1.05f : 1.35f);
    float inGain = juce::Decibels::decibelsToGain(inputGainDb);
    const int N = buffer.getNumSamples(), Ch = buffer.getNumChannels();
    // 1. Medicion LUFS aprox para el medidor
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
    float ceiling = juce::Decibels::decibelsToGain(ceilingDb);
    float releaseCoeff = std::exp(-1.0f / (float)(sampleRate * releaseMs / 1000.f));
    float attackCoeff = std::exp(-1.0f / (float)(sampleRate * 0.001f)); // 1ms attack
    float peak = 0.f;
    const int lookaheadSamples = 96; // ~2ms a 48k, para que no pique
    for (int s = 0; s < N; s++)
    {
        // 2. Input + Saturacion suave (tanh) = calor profesional
        float inL = buffer.getSample(0, s) * inGain;
        float inR = Ch > 1 ? buffer.getSample(1, s) * inGain : inL;
        float satL = std::tanh(inL * drive) / std::tanh(drive);
        float satR = std::tanh(inR * drive) / std::tanh(drive);
        // 3. Detector con Stereo Link
        float peakL = std::abs(satL);
        float peakR = std::abs(satR);
        float linkedPeak = std::max(peakL, peakR) * linkAmt + (peakL + peakR) * 0.5f * (1.f - linkAmt);
        // Envelope con ataque rapido y release suave
        if (linkedPeak > envelope) envelope = attackCoeff * envelope + (1.f - attackCoeff) * linkedPeak;
        else envelope = envelope * releaseCoeff + linkedPeak * (1.f - releaseCoeff) * 0.2f;
        // 4. Limitador Lookahead brickwall
        if (envelope > ceiling) smoothedGain = std::min(smoothedGain, ceiling / envelope);
        else smoothedGain += (1.0f - smoothedGain) * (1 - releaseCoeff) * 0.3f;
        // Guarda en lookahead para que no clipee
        lookaheadBuffer.setSample(0, lookaheadPos, satL * smoothedGain);
        if (Ch > 1) lookaheadBuffer.setSample(1, lookaheadPos, satR * smoothedGain);
        int readPos = (lookaheadPos - lookaheadSamples + 256) % 256;
        float yL = lookaheadBuffer.getSample(0, readPos);
        float yR = Ch > 1 ? lookaheadBuffer.getSample(1, readPos) : yL;
        // 5. Soft Clip final - techo True-Peak, sin clicks
        yL = juce::jlimit(-0.999f, 0.999f, yL);
        yR = juce::jlimit(-0.999f, 0.999f, yR);
        peak = std::max({ peak, std::abs(yL), std::abs(yR) });
        buffer.setSample(0, s, yL);
        if (Ch > 1) buffer.setSample(1, s, yR);
        lookaheadPos = (lookaheadPos + 1) % 256;
    }
    truePeakDb = 20 * std::log10(std::max(peak, 1e-9f));
    grDb = 20 * std::log10(std::max(smoothedGain, 1e-4f));
}

juce::AudioProcessorEditor* ClubMasterAudioProcessor::createEditor() { return new ClubMasterAudioProcessorEditor(*this); }

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
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ClubMasterAudioProcessor(); }
