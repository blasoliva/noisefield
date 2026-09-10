#include "gui/SettingsComponent.h"

namespace noisefield::gui
{

SettingsComponent::SettingsComponent(engine::AudioEngine& engine)
    : engine_(engine),
      deviceSelector_(engine.deviceManager(), 0, 0, 1, 2, false, false, true, false)
{
    limiterHeading_.setText("Master", juce::dontSendNotification);
    limiterHeading_.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    limiterHeading_.setColour(juce::Label::textColourId, juce::Colour(0xffb9c0c8));
    addAndMakeVisible(limiterHeading_);

    limiterButton_.setToggleState(
        engine_.parameters().limiterEnabled.load(std::memory_order_relaxed),
        juce::dontSendNotification);
    limiterButton_.onClick = [this]
    {
        engine_.parameters().limiterEnabled.store(limiterButton_.getToggleState(),
                                                  std::memory_order_relaxed);
    };
    addAndMakeVisible(limiterButton_);

    limiterHint_.setText("Below ~ -1 dBFS it does nothing; above that it compresses peaks so "
                         "the output never clips.",
                         juce::dontSendNotification);
    limiterHint_.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    limiterHint_.setJustificationType(juce::Justification::topLeft);
    limiterHint_.setMinimumHorizontalScale(1.0f);
    addAndMakeVisible(limiterHint_);

    deviceHeading_.setText("Audio device", juce::dontSendNotification);
    deviceHeading_.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    deviceHeading_.setColour(juce::Label::textColourId, juce::Colour(0xffb9c0c8));
    addAndMakeVisible(deviceHeading_);

    addAndMakeVisible(deviceSelector_);

    setSize(480, 540);
}

void SettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SettingsComponent::resized()
{
    auto area = getLocalBounds().reduced(16);

    limiterHeading_.setBounds(area.removeFromTop(20));
    limiterButton_.setBounds(area.removeFromTop(28));
    limiterHint_.setBounds(area.removeFromTop(40));
    area.removeFromTop(12);

    deviceHeading_.setBounds(area.removeFromTop(20));
    area.removeFromTop(4);
    deviceSelector_.setBounds(area);
}

} // namespace noisefield::gui
