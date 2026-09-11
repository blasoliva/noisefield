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

    /// A `ToggleButton` with no text (the section header switches: Tone/Noise "Enabled") draws
    /// as a bare track-and-thumb switch instead of a tickbox; one that still has a label (e.g.
    /// the Settings window's "Soft limiter") keeps the normal tickbox + text.
    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override
    {
        if (button.getButtonText().isNotEmpty())
        {
            LookAndFeel_V4::drawToggleButton(
                g, button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
            return;
        }

        const auto bounds = button.getLocalBounds().toFloat();
        const float h = bounds.getHeight();
        const auto track = bounds.withSizeKeepingCentre(juce::jmin(bounds.getWidth(), h * 1.8f), h);

        const bool on = button.getToggleState();
        const auto accent = juce::Colour(0xff5fc7ea);

        g.setColour(on ? accent.withAlpha(0.18f) : juce::Colour(0xff232830));
        g.fillRoundedRectangle(track, h * 0.5f);
        g.setColour(on ? accent : juce::Colour(0xff2b2f38));
        g.drawRoundedRectangle(track.reduced(0.5f), h * 0.5f, 1.0f);

        const float thumbD = h - 6.0f;
        const float thumbX = on ? track.getRight() - thumbD - 2.0f : track.getX() + 2.0f;
        g.setColour(on ? accent : juce::Colour(0xff6a6f78));
        g.fillEllipse(thumbX, track.getY() + 3.0f, thumbD, thumbD);
    }

    /// The stock LookAndFeel_V4 linear slider insets its track by the thumb radius, so the
    /// usable travel (and how "full" the track looks) shifts with the slider's own width and
    /// range -- most visible on `LinearHorizontal` sliders back to back at different widths.
    /// Draw a track that always spans the full [x, x+width) given to us, filled from the left
    /// up to the thumb, with a plain circular thumb -- independent of slider position.
    void drawLinearSlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider(
                g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            return;
        }

        const auto accent = juce::Colour(0xff5fc7ea);
        const auto colour = slider.isEnabled() ? accent : juce::Colour(0xff4a4f58);

        const float left = static_cast<float>(x);
        const float right = static_cast<float>(x + width);
        const float trackY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        constexpr float kTrackH = 4.0f;

        g.setColour(juce::Colour(0xff232830));
        g.fillRoundedRectangle(
            left, trackY - kTrackH * 0.5f, right - left, kTrackH, kTrackH * 0.5f);

        g.setColour(colour);
        g.fillRoundedRectangle(
            left, trackY - kTrackH * 0.5f, sliderPos - left, kTrackH, kTrackH * 0.5f);

        constexpr float kThumbD = 14.0f;
        g.setColour(colour);
        g.fillEllipse(sliderPos - kThumbD * 0.5f, trackY - kThumbD * 0.5f, kThumbD, kThumbD);
        g.setColour(juce::Colour(0xff16181c));
        g.drawEllipse(sliderPos - kThumbD * 0.5f, trackY - kThumbD * 0.5f, kThumbD, kThumbD, 2.0f);
    }
};

} // namespace noisefield::app
