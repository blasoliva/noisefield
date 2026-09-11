#include "gui/Oscilloscope.h"

#include <utility>

namespace noisefield::gui
{

Oscilloscope::Oscilloscope(std::function<void(float*, int)> fillLatest)
    : fillLatest_(std::move(fillLatest)), window_(static_cast<size_t>(kWindow), 0.0f)
{
    setInterceptsMouseClicks(false, false);
}

void Oscilloscope::visibilityChanged()
{
    if (isVisible())
        startTimerHz(30);
    else
        stopTimer();
}

void Oscilloscope::timerCallback()
{
    repaint();
}

void Oscilloscope::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff0c0d10));
    g.fillRoundedRectangle(bounds, 3.0f);

    // Faint vertical divisions, like an instrument's graticule.
    g.setColour(juce::Colour(0x14ffffff));
    constexpr int kDivisions = 5;
    for (int i = 1; i < kDivisions; ++i)
    {
        const float x = bounds.getX() + bounds.getWidth() * (static_cast<float>(i) / kDivisions);
        g.drawVerticalLine(juce::roundToInt(x), bounds.getY(), bounds.getBottom());
    }

    // Centre line.
    g.setColour(juce::Colour(0x22ffffff));
    const float midY = bounds.getCentreY();
    g.drawHorizontalLine(juce::roundToInt(midY), bounds.getX(), bounds.getRight());

    if (fillLatest_)
        fillLatest_(window_.data(), kWindow);

    // Trigger: first rising zero-crossing in the part of the window we won't run past.
    int start = 0;
    for (int i = 1; i < kWindow - kDisplay; ++i)
    {
        if (window_[static_cast<size_t>(i) - 1] < 0.0f && window_[static_cast<size_t>(i)] >= 0.0f)
        {
            start = i;
            break;
        }
    }

    juce::Path path;
    const float amp = bounds.getHeight() * 0.5f * 0.9f;
    for (int i = 0; i < kDisplay; ++i)
    {
        const float x = bounds.getX() + bounds.getWidth() * (static_cast<float>(i) /
                                                             static_cast<float>(kDisplay - 1));
        const float s = juce::jlimit(-1.0f, 1.0f, window_[static_cast<size_t>(start + i)]);
        const float y = midY - s * amp;
        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }

    g.setColour(juce::Colour(0xff5fc7ea));
    g.strokePath(path, juce::PathStrokeType(1.5f));

    g.setColour(juce::Colour(0x22ffffff));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
}

} // namespace noisefield::gui
