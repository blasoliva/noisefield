#pragma once

#include "engine/AudioEngine.h"

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace noisefield::gui
{

/// Content of the detached "Settings" window: the master soft limiter plus the full audio
/// device selector (output, sample rate, buffer size, active channels, advanced options).
class SettingsComponent final : public juce::Component
{
public:
    explicit SettingsComponent(engine::AudioEngine& engine);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    engine::AudioEngine& engine_;

    juce::Label limiterHeading_;
    juce::ToggleButton limiterButton_{"Soft limiter"};
    juce::Label limiterHint_;

    juce::Label deviceHeading_;
    juce::AudioDeviceSelectorComponent deviceSelector_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
};

} // namespace noisefield::gui
