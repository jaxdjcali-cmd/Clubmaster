#include "PluginProcessor.h"
#include "PluginEditor.h"

ClubMasterAudioProcessorEditor::ClubMasterAudioProcessorEditor(ClubMasterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(720, 480);
    startTimerHz(20);
    // Knob grande
    addAndMakeVisible(inputKnob);
    inputKnob.setRange(-12, 12, 0.1);
    inputKnob.setTextValueSuffix(" dB");
    inputKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00e5ff));
    inputKnob.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    // Botones de modo con colores pro
    addAndMakeVisible(modeSquash);
    addAndMakeVisible(modeClub);
    addAndMakeVisible(modeStream);
    modeSquash.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffffa500));
    modeClub.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffaa00ff));
    modeStream.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00c8ff));
    modeSquash.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    modeClub.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    modeStream.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    modeSquash.onClick = [this] { audioProcessor.apvts.getParameter("mode")->setValueNotifyingHost(0.0f); updateModeButtons(); };
    modeClub.onClick = [this] { audioProcessor.apvts.getParameter("mode")->setValueNotifyingHost(0.5f); updateModeButtons(); };
    modeStream.onClick = [this] { audioProcessor.apvts.getParameter("mode")->setValueNotifyingHost(1.0f); updateModeButtons(); };
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
    addAndMakeVisible(titleLabel);
    titleLabel.setText("CLUB MASTER", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00e5ff));
    lufsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    peakLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    grLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffcc00));
    a1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "input", inputKnob);
    a2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "ceiling", ceilingSlider);
    a3 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "release", releaseSlider);
    a4 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "link", linkSlider);
    aHq = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "quality", hqButton);
    updateModeButtons();
}
ClubMasterAudioProcessorEditor::~ClubMasterAudioProcessorEditor() { stopTimer(); }

void ClubMasterAudioProcessorEditor::updateModeButtons()
{
    int m = (int)*audioProcessor.apvts.getRawParameterValue("mode");
    modeSquash.setToggleState(m == 0, juce::dontSendNotification);
    modeClub.setToggleState(m == 1, juce::dontSendNotification);
    modeStream.setToggleState(m == 2, juce::dontSendNotification);
}

void ClubMasterAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fondo negro pro con textura
    g.fillAll(juce::Colour(0xff0a0a0f));
    g.setColour(juce::Colour(0xff1a1a22));
    g.fillRoundedRectangle(10, 10, 700, 460, 12);
    g.setColour(juce::Colour(0xff2a2a35));
    g.drawRoundedRectangle(10, 10, 700, 460, 12, 2);
    // Titulo
    g.setColour(juce::Colour(0xff00e5ff));
    g.setFont(juce::Font(32.f, juce::Font::bold));
    g.drawText("CLUB MASTER", 25, 18, 300, 35, juce::Justification::left);
    g.setColour(juce::Colours::lightgrey);
    g.setFont(12.f);
    g.drawText("BY JAX DJ LATINO  -  MASTERING PARA DJs", 25, 52, 400, 16, juce::Justification::left);
    g.setColour(juce::Colour(0xffffcc00));
    g.setFont(11.f);
    g.drawText("YOUR SIGNAL INPUT", 470, 18, 200, 14, juce::Justification::right);
    // Medidores estilo Mashup Master - barras de colores
    auto drawMeter = [&](int x, int y, int w, float valueDb, float minDb, float maxDb, juce::String label)
    {
        g.setColour(juce::Colour(0xff111118));
        g.fillRoundedRectangle(x, y, w, 22, 4);
        float norm = juce::jlimit(0.f, 1.f, (valueDb - minDb) / (maxDb - minDb));
        int fillW = (int)(w * norm);
        for (int i = 0; i < fillW; i += 4)
        {
            float pos = (float)i / (float)w;
            juce::Colour c = pos < 0.6f ? juce::Colour(0xff00ff88) : (pos < 0.85f ? juce::Colour(0xffffcc00) : juce::Colour(0xffff2222));
            g.setColour(c);
            g.fillRect(x + i, y + 2, 2, 18);
        }
        g.setColour(juce::Colours::white);
        g.setFont(11.f);
        g.drawText(label, x, y - 18, w, 14, juce::Justification::left);
        g.drawText(juce::String(valueDb, 1) + " dB", x + w - 70, y - 18, 70, 14, juce::Justification::right);
    };
    drawMeter(25, 110, 450, meterLUFS, -30, 0, "LUFS INTEGRATED");
    drawMeter(25, 160, 450, meterPeak, -30, 0, "TRUE PEAK");
    drawMeter(25, 210, 450, meterGR, -12, 0, "GAIN REDUCTION");
    // Etiquetas de controles
    g.setColour(juce::Colours::lightgrey);
    g.setFont(11.f);
    g.drawText("CEILING", 500, 270, 180, 14, juce::Justification::left);
    g.drawText("RELEASE", 500, 320, 180, 14, juce::Justification::left);
    g.drawText("STEREO LINK", 500, 370, 180, 14, juce::Justification::left);
    g.setColour(juce::Colour(0xff00e5ff));
    g.setFont(13.f);
    g.drawText("INPUT", 75, 415, 150, 16, juce::Justification::centred);
}

void ClubMasterAudioProcessorEditor::resized()
{
    titleLabel.setBounds(0, 0, 0, 0); // lo dibujamos en paint
    inputKnob.setBounds(50, 260, 190, 170);
    modeSquash.setBounds(25, 270, 145, 32);
    modeClub.setBounds(180, 270, 145, 32);
    modeStream.setBounds(335, 270, 145, 32);
    ceilingSlider.setBounds(500, 290, 180, 28);
    releaseSlider.setBounds(500, 340, 180, 28);
    linkSlider.setBounds(500, 390, 180, 28);
    hqButton.setBounds(500, 425, 60, 22);
    lufsLabel.setBounds(0, 0, 0, 0);
    peakLabel.setBounds(0, 0, 0, 0);
    grLabel.setBounds(0, 0, 0, 0);
}
void ClubMasterAudioProcessorEditor::timerCallback()
{
    meterLUFS = audioProcessor.getLUFS();
    meterPeak = audioProcessor.getTruePeak();
    meterGR = audioProcessor.getGR();
    repaint();
}
