#pragma once

#include "gui/Oscilloscope.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace noisefield::gui
{

/// Content of the detached "Scope" window: the master-output oscilloscope with a heading.
/// (The FFT spectrum view, NF-060, will join it here later.)
class ScopeView final : public juce::Component
{
public:
    explicit ScopeView(std::function<void(float*, int)> fillLatest);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label heading_;
    Oscilloscope oscilloscope_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeView)
};

} // namespace noisefield::gui
