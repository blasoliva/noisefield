#include "gui/LevelMeter.h"

#include "dsp/Gain.h"

#include <cmath>

namespace noisefield::gui
{

LevelMeter::LevelMeter()
{
    setInterceptsMouseClicks(false, false);
}

float LevelMeter::dbToProportion(float decibels)
{
    if (decibels <= kFloorDb)
        return 0.0f;
    return juce::jlimit(0.0f, 1.0f, (decibels - kFloorDb) / -kFloorDb);
}

void LevelMeter::setLevel(float peak, float rms)
{
    const auto now = juce::Time::currentTimeMillis();
    const auto elapsed =
        lastUpdateMs_ == 0 ? 16 : juce::jlimit<juce::int64>(1, 200, now - lastUpdateMs_);
    lastUpdateMs_ = now;

    // RMS follows quickly; peak holds then decays ~ 12 dB/s so short transients stay visible.
    displayRms_ = juce::jmax(rms, displayRms_ * 0.6f);
    const float decay = std::pow(10.0f, -0.05f * 12.0f * (static_cast<float>(elapsed) / 1000.0f));
    displayPeak_ = juce::jmax(peak, displayPeak_ * decay);

    repaint();
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff0c0d10));
    g.fillRoundedRectangle(bounds, 3.0f);

    const float rmsProportion = dbToProportion(noisefield::dsp::gainToDb(displayRms_, kFloorDb));
    const float peakProportion = dbToProportion(noisefield::dsp::gainToDb(displayPeak_, kFloorDb));

    auto fill = bounds.withWidth(bounds.getWidth() * rmsProportion);
    juce::ColourGradient gradient(juce::Colour(0xff38bdf8),
                                  bounds.getX(),
                                  0.0f,
                                  juce::Colour(0xffef4444),
                                  bounds.getRight(),
                                  0.0f,
                                  false);
    gradient.addColour(0.8, juce::Colour(0xfffacc15));
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(fill, 3.0f);

    if (peakProportion > 0.0f)
    {
        const float x = bounds.getX() + bounds.getWidth() * peakProportion;
        g.setColour(displayPeak_ >= 1.0f ? juce::Colour(0xffef4444) : juce::Colour(0xffe6e8ec));
        g.fillRect(x - 1.0f, bounds.getY(), 2.0f, bounds.getHeight());
    }

    g.setColour(juce::Colour(0x22ffffff));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);
}

} // namespace noisefield::gui
