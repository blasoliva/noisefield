#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace noisefield::app
{

/// Dark theme for the whole application. Colour choices will be refined alongside the real
/// UI (M3); for now it just gives a consistent dark surface.
class NoisefieldLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    NoisefieldLookAndFeel() : juce::LookAndFeel_V4(juce::LookAndFeel_V4::getDarkColourScheme())
    {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff16181c));
        setColour(juce::Label::textColourId, juce::Colour(0xffe6e8ec));
    }
};

} // namespace noisefield::app
