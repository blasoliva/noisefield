#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace noisefield::gui
{

/// A rounded-rect background panel that visually groups one section of the main window
/// (Monitor, Preset, Tone, Noise, Master, Session timer). Purely decorative: add it as a
/// sibling behind the section's real controls (`addAndMakeVisible` it first, or call
/// `toBack()`) and give it the same bounds as that section; the caller still positions its
/// own child components inside, padded from those bounds.
class SectionCard final : public juce::Component
{
public:
    SectionCard() = default;

    void paint(juce::Graphics& g) override
    {
        constexpr float kRadius = 10.0f;
        const auto bounds = getLocalBounds().toFloat();

        g.setColour(juce::Colour(0xff1b1e24));
        g.fillRoundedRectangle(bounds, kRadius);
        g.setColour(juce::Colour(0xff2b2f38));
        g.drawRoundedRectangle(bounds.reduced(0.5f), kRadius, 1.0f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionCard)
};

} // namespace noisefield::gui
