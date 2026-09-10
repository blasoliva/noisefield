#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace noisefield::gui
{

/// Time-domain view of the master output. Pulls a window of recent samples on a timer and
/// draws it aligned to a rising zero-crossing so a steady tone stands still.
class Oscilloscope final : public juce::Component, private juce::Timer
{
public:
    /// `fillLatest(dst, count)` must copy the most recent `count` samples into `dst`.
    explicit Oscilloscope(std::function<void(float*, int)> fillLatest);

    void paint(juce::Graphics& g) override;
    void visibilityChanged() override;

private:
    void timerCallback() override;

    static constexpr int kWindow = 2048;  // samples fetched per frame
    static constexpr int kDisplay = 1024; // samples actually drawn

    std::function<void(float*, int)> fillLatest_;
    std::vector<float> window_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oscilloscope)
};

} // namespace noisefield::gui
