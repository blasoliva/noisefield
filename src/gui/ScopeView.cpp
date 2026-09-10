#include "gui/ScopeView.h"

#include <utility>

namespace noisefield::gui
{

ScopeView::ScopeView(std::function<void(float*, int)> fillLatest)
    : oscilloscope_(std::move(fillLatest))
{
    heading_.setText("Oscilloscope (master output)", juce::dontSendNotification);
    heading_.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    heading_.setColour(juce::Label::textColourId, juce::Colour(0xffb9c0c8));
    addAndMakeVisible(heading_);
    addAndMakeVisible(oscilloscope_);

    setSize(580, 240);
}

void ScopeView::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ScopeView::resized()
{
    auto area = getLocalBounds().reduced(16);
    heading_.setBounds(area.removeFromTop(20));
    area.removeFromTop(6);
    oscilloscope_.setBounds(area);
}

} // namespace noisefield::gui
