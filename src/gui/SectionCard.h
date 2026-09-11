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

    /// The Master card gets a faint accent tint and border to read as the primary control.
    void setHighlighted(bool highlighted)
    {
        if (highlighted_ != highlighted)
        {
            highlighted_ = highlighted;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        constexpr float kRadius = 10.0f;
        const auto bounds = getLocalBounds().toFloat();

        if (highlighted_)
        {
            g.setGradientFill(juce::ColourGradient(juce::Colour(0x2a5fc7ea),
                                                    bounds.getX(),
                                                    bounds.getY(),
                                                    juce::Colour(0x005fc7ea),
                                                    bounds.getX(),
                                                    bounds.getY() + bounds.getHeight() * 0.7f,
                                                    false));
            g.fillRoundedRectangle(bounds, kRadius);
            g.setColour(juce::Colour(0xff1b1e24));
            g.fillRoundedRectangle(bounds.reduced(1.0f), kRadius - 1.0f);
            g.setColour(juce::Colour(0xff5fc7ea));
        }
        else
        {
            g.setColour(juce::Colour(0xff1b1e24));
            g.fillRoundedRectangle(bounds, kRadius);
            g.setColour(juce::Colour(0xff2b2f38));
        }
        g.drawRoundedRectangle(bounds.reduced(0.5f), kRadius, 1.0f);
    }

private:
    bool highlighted_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionCard)
};

} // namespace noisefield::gui
