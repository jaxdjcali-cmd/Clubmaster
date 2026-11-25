#include "PluginProcessor.h"
#include "PluginEditor.h"

ClubMasterAudioProcessorEditor::ClubMasterAudioProcessorEditor(ClubMasterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(800, 520);
    startTimerHz(30);
    // KNOB - sin caja de texto fea, solo el knob
    addAndMakeVisible(inputKnob);
    inputKnob.setRange(-12, 12, 0.1);
    inputKnob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00e5ff));
    inputKnob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a2a35));
    inputKnob.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    // BOTONES MODO - estilo pastilla pro
    addAndMakeVisible(modeSquash);
    addAndMakeVisible(modeClub);
    addAndMakeVisible(modeStream);
    for (auto* b : { &modeSquash, &modeClub, &modeStream })
    {
        b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e1e28));
        b->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00e5ff));
        b->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffaaaaaa));
        b->setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    }
    modeSquash.onClick = [this] { audioProcessor.apvts.getParameter("mode")->setValueNotifyingHost(0.0f); updateModeButtons(); };
    modeClub.onClick = [this] { audioProcessor.apvts.getParameter("mode")->setValueNotifyingHost(0.5f); updateModeButtons(); };
    modeStream.onClick = [this] { audioProcessor.apvts.getParameter("mode")->setValueNotifyingHost(1.0f); updateModeButtons(); };
    addAndMakeVisible(ceilingSlider);
    ceilingSlider.setRange(-3, -0.1, 0.1);
    ceilingSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00e5ff));
    ceilingSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible(releaseSlider);
    releaseSlider.setRange(20, 500, 1);
    releaseSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00e5ff));
    releaseSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible(linkSlider);
    linkSlider.setRange(0, 1, 0.01);
    linkSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00e5ff));
    linkSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible(hqButton);
    hqButton.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff00e5ff));
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
    // Color activo segun modo, como en el video
    modeSquash.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffffa500));
    modeClub.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffaa00ff));
    modeStream.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00c8ff));
}

void ClubMasterAudioProcessorEditor::paint(juce::Graphics& g)
{
    // FONDO - metal oscuro cepillado, no gris plano de novato
    g.fillAll(juce::Colour(0xff08080c));
    auto bg = juce::Rectangle<float>(8, 8, 784, 504);
    juce::ColourGradient grad(juce::Colour(0xff23232e), 0, 0, juce::Colour(0xff0e0e14), 0, 520, false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bg, 14);
    g.setColour(juce::Colour(0xff3a3a48));
    g.drawRoundedRectangle(bg, 14, 1.5f);
    // CABECERA
    g.setColour(juce::Colour(0xff00e5ff));
    g.setFont(juce::Font(34.f, juce::Font::bold));
    g.drawText("CLUB", 28, 18, 120, 38, juce::Justification::left);
    g.setColour(juce::Colour(0xffffa500));
    g.drawText("MASTER", 118, 18, 180, 38, juce::Justification::left);
    g.setColour(juce::Colour(0xff888899));
    g.setFont(11.f);
    g.drawText("BY JAX DJ LATINO", 28, 56, 200, 14, juce::Justification::left);
    g.setColour(juce::Colour(0xffffcc00));
    g.setFont(juce::Font(11.f, juce::Font::bold));
    g.drawText("YOUR SIGNAL INPUT", 560, 22, 200, 14, juce::Justification::right);
    g.setColour(juce::Colour(0xff666677));
    g.setFont(10.f);
    g.drawText("PROFESSIONAL MASTERING FOR DJs", 560, 38, 200, 14, juce::Justification::right);
    // MEDIDORES - estilo LED del video, con segmentos
    auto drawLEDMeter = [&](int x, int y, int w, float valueDb, float minDb, float maxDb, juce::String label, juce::String valueText)
    {
        g.setColour(juce::Colour(0xff000000));
        g.fillRoundedRectangle(x, y, w, 26, 5);
        g.setColour(juce::Colour(0xff2a2a35));
        g.drawRoundedRectangle(x, y, w, 26, 5, 1);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(11.f, juce::Font::bold));
        g.drawText(label, x, y - 18, 180, 14, juce::Justification::left);
        g.setColour(juce::Colour(0xffffcc00));
        g.drawText(valueText, x + w - 80, y - 18, 80, 14, juce::Justification::right);
        int segs = 48;
        float norm = juce::jlimit(0.f, 1.f, (valueDb - minDb) / (maxDb - minDb));
        int lit = (int)(segs * norm);
        for (int i = 0; i < segs; ++i)
        {
            float pos = (float)i / (float)segs;
            juce::Colour c = juce::Colour(0xff1a2a1a);
            if (i < lit) c = pos < 0.6f ? juce::Colour(0xff00ff88) : (pos < 0.85f ? juce::Colour(0xffffcc00) : juce::Colour(0xffff2222));
            float sx = x + 6 + i * ((w - 12) / (float)segs);
            g.setColour(c);
            g.fillRoundedRectangle(sx, y + 5, (w - 12) / (float)segs - 1.5f, 16, 2);
        }
    };
    drawLEDMeter(28, 110, 520, smoothLUFS, -30, 0, "LUFS INTEGRATED", juce::String(smoothLUFS, 1) + " LUFS");
    drawLEDMeter(28, 162, 520, smoothPeak, -30, 0, "TRUE PEAK", juce::String(smoothPeak, 1) + " dBTP");
    drawLEDMeter(28, 214, 520, smoothGR, -12, 0, "GAIN REDUCTION", juce::String(smoothGR, 1) + " dB");
    // SEPARADOR
    g.setColour(juce::Colour(0xff2a2a35));
    g.fillRect(28, 268, 520, 1);
    // ETIQUETAS INFERIORES
    g.setColour(juce::Colour(0xffaaaaaa));
    g.setFont(10.f);
    g.drawText("INPUT", 110, 480, 120, 14, juce::Justification::centred);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.f, juce::Font::bold));
    float inDb = *audioProcessor.apvts.getRawParameterValue("input");
    g.drawText(juce::String(inDb, 1) + " dB", 110, 496, 120, 16, juce::Justification::centred);
    g.setColour(juce::Colour(0xffaaaaaa));
    g.setFont(10.f);
    g.drawText("CEILING  " + juce::String(*audioProcessor.apvts.getRawParameterValue("ceiling"), 1) + " dB", 580, 300, 190, 14, juce::Justification::left);
    g.drawText("RELEASE  " + juce::String((int)*audioProcessor.apvts.getRawParameterValue("release")) + " ms", 580, 360, 190, 14, juce::Justification::left);
    g.drawText("STEREO LINK  " + juce::String(*audioProcessor.apvts.getRawParameterValue("link"), 2), 580, 420, 190, 14, juce::Justification::left);
}

void ClubMasterAudioProcessorEditor::resized()
{
    // LAYOUT SIN SOLAPES - cada cosa en su zona
    // Medidores: y 110-240 (se dibujan en paint, no hay controles ahi)
    // Botones modo: fila sola y=282
    modeSquash.setBounds(28, 282, 165, 36);
    modeClub.setBounds(200, 282, 165, 36);
    modeStream.setBounds(372, 282, 165, 36);
    // Knob: abajo izquierda SOLO, y=340
    inputKnob.setBounds(95, 340, 150, 135);
    // Controles derecha: cada uno en su fila
    ceilingSlider.setBounds(580, 318, 190, 26);
    releaseSlider.setBounds(580, 378, 190, 26);
    linkSlider.setBounds(580, 438, 190, 26);
    hqButton.setBounds(580, 470, 70, 22);
}
void ClubMasterAudioProcessorEditor::timerCallback()
{
    float tLUFS = audioProcessor.getLUFS();
    float tPeak = audioProcessor.getTruePeak();
    float tGR = audioProcessor.getGR();
    // Suavizado para que los medidores no salten como de novato
    smoothLUFS = 0.7f * smoothLUFS + 0.3f * tLUFS;
    smoothPeak = 0.7f * smoothPeak + 0.3f * tPeak;
    smoothGR = 0.7f * smoothGR + 0.3f * tGR;
    repaint();
}
