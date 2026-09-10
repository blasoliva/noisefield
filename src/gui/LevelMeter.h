#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace noisefield::gui
{

/// Horizontal master-level meter: a filled RMS bar with a brighter peak-hold marker, on a
/// dBFS scale. Fed from the GUI timer via setLevel(); it does no polling of its own.
class LevelMeter final : public juce::Component
{
public:
    LevelMeter();

    /// `peak` and `rms` are linear magnitudes in [0, 1+].
    void setLevel(float peak, float rms);

    void paint(juce::Graphics& g) override;

private:
    static constexpr float kFloorDb = -60.0f;

    static float dbToProportion(float decibels);

    float displayRms_ = 0.0f;  // smoothed for readability
    float displayPeak_ = 0.0f; // decays slowly
    juce::int64 lastUpdateMs_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

} // namespace noisefield::gui
