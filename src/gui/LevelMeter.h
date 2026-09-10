#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace noisefield::gui
{

/// Horizontal master-level meter on a dBFS scale: a filled RMS bar with a brighter peak-hold
/// marker, faint scale ticks, and a clip latch that lights at 0 dBFS and clears itself after a
/// few quiet seconds. Fed from the GUI timer via setLevel(); it does no polling of its own.
class LevelMeter final : public juce::Component
{
public:
    LevelMeter();

    /// `peak` and `rms` are linear magnitudes in [0, 1+].
    void setLevel(float peak, float rms);

    /// Current (decaying) peak level in dBFS, for a numeric readout elsewhere.
    [[nodiscard]] float currentPeakDb() const;

    void paint(juce::Graphics& g) override;

private:
    static constexpr float kFloorDb = -60.0f;

    static float dbToProportion(float decibels);

    float displayRms_ = 0.0f;  // smoothed for readability
    float displayPeak_ = 0.0f; // decays slowly
    bool clipped_ = false;
    juce::int64 lastUpdateMs_ = 0;
    juce::int64 lastClipMs_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

} // namespace noisefield::gui
