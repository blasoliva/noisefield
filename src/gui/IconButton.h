#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <utility>

namespace noisefield::gui
{

/// A button styled like `juce::TextButton` (same background/border, same toggle-state
/// colouring) that draws a small vector icon instead of a text label. See `icons::` below for
/// the icons used in the transport bar.
class IconButton final : public juce::Button
{
public:
    explicit IconButton(std::function<void(juce::Graphics&, juce::Rectangle<float>)> drawIcon)
        : juce::Button({}), drawIcon_(std::move(drawIcon))
    {
    }

    void paintButton(juce::Graphics& g,
                     bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override
    {
        auto& lf = getLookAndFeel();
        lf.drawButtonBackground(
            g,
            *this,
            findColour(getToggleState() ? juce::TextButton::buttonOnColourId
                                        : juce::TextButton::buttonColourId),
            shouldDrawButtonAsHighlighted,
            shouldDrawButtonAsDown);

        const auto bounds = getLocalBounds().toFloat();
        const float size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.44f;
        const auto iconBounds = bounds.withSizeKeepingCentre(size, size);

        g.setColour(getToggleState() ? juce::Colour(0xff5fc7ea) : juce::Colour(0xffb9c0c8));
        if (drawIcon_)
            drawIcon_(g, iconBounds);
    }

private:
    std::function<void(juce::Graphics&, juce::Rectangle<float>)> drawIcon_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IconButton)
};

/// Icon-drawing functions for the transport bar's utility buttons (Scope / Timer / Guide /
/// Settings). Each draws into normalised [0,1] space mapped onto `bounds`, stroked in the
/// colour the caller already set.
namespace icons
{
inline juce::Point<float> lerp(juce::Rectangle<float> bounds, float nx, float ny)
{
    return {bounds.getX() + nx * bounds.getWidth(), bounds.getY() + ny * bounds.getHeight()};
}

inline void scope(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    juce::Path p;
    p.startNewSubPath(lerp(bounds, 0.02f, 0.5f));
    p.lineTo(lerp(bounds, 0.25f, 0.5f));
    p.lineTo(lerp(bounds, 0.4f, 0.12f));
    p.lineTo(lerp(bounds, 0.62f, 0.88f));
    p.lineTo(lerp(bounds, 0.77f, 0.5f));
    p.lineTo(lerp(bounds, 1.0f, 0.5f));
    g.strokePath(p, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved,
                                         juce::PathStrokeType::rounded));
}

inline void timer(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const auto centre = lerp(bounds, 0.5f, 0.55f);
    const float radius = bounds.getWidth() * 0.42f;
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.6f);

    juce::Path hands;
    hands.startNewSubPath(centre);
    hands.lineTo(centre.x, centre.y - radius * 0.62f);
    hands.startNewSubPath(centre);
    hands.lineTo(centre.x + radius * 0.5f, centre.y);
    g.strokePath(hands, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
}

inline void guide(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const float gap = bounds.getWidth() * 0.045f;
    const auto left =
        juce::Rectangle<float>(bounds.getX(), bounds.getY(), bounds.getWidth() * 0.5f - gap,
                               bounds.getHeight())
            .withTop(bounds.getY() + bounds.getHeight() * 0.08f)
            .withBottom(bounds.getBottom() - bounds.getHeight() * 0.05f);
    const auto right = left.withX(bounds.getCentreX() + gap);

    g.drawRoundedRectangle(left, 1.5f, 1.4f);
    g.drawRoundedRectangle(right, 1.5f, 1.4f);
}

inline void settings(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const auto centre = bounds.getCentre();
    const float hubR = bounds.getWidth() * 0.16f;
    g.drawEllipse(centre.x - hubR, centre.y - hubR, hubR * 2.0f, hubR * 2.0f, 1.5f);

    constexpr int kTeeth = 8;
    const float innerR = bounds.getWidth() * 0.3f;
    const float outerR = bounds.getWidth() * 0.46f;
    for (int i = 0; i < kTeeth; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * static_cast<float>(i) / kTeeth;
        juce::Line<float> tooth(centre.getPointOnCircumference(innerR, angle),
                                centre.getPointOnCircumference(outerR, angle));
        g.drawLine(tooth, 1.5f);
    }
}
} // namespace icons

} // namespace noisefield::gui
