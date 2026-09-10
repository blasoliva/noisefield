#pragma once

#include "engine/AudioEngine.h"
#include "gui/LevelMeter.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace noisefield::app
{

/// Root content component for the M2 MVP: transport, a tone source (frequency + level), a
/// white-noise source (level) and a master level, with a level meter. The soft limiter and
/// audio-device settings live in a detached Settings window; the user guide in a detached
/// Guide window. Control values persist between runs.
class MainComponent final : public juce::Component, private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    void loadSettings();
    void saveSettings();
    void pushAllParametersToEngine();
    void openSettingsWindow();
    void openGuideWindow();

    engine::EngineParameters& params()
    {
        return engine_.parameters();
    }

    juce::ApplicationProperties appProperties_;
    engine::AudioEngine engine_;

    juce::TextButton playButton_{"Play"};
    juce::TextButton masterMuteButton_{"Mute"};
    juce::TextButton guideButton_{"Guide"};
    juce::TextButton settingsButton_{"Settings"};
    gui::LevelMeter meter_;

    juce::Label toneHeading_;
    juce::ToggleButton toneEnableButton_{"Enabled"};
    juce::Slider frequencySlider_;
    juce::Slider toneGainSlider_;

    juce::Label noiseHeading_;
    juce::ToggleButton noiseEnableButton_{"Enabled"};
    juce::ComboBox noiseColourBox_;
    juce::Slider noiseGainSlider_;
    juce::TextButton reseedButton_{"Re-seed"};

    juce::Label masterHeading_;
    juce::Slider masterGainSlider_;

    juce::Label statusLabel_;

    std::unique_ptr<juce::DocumentWindow> settingsWindow_;
    std::unique_ptr<juce::DocumentWindow> guideWindow_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace noisefield::app
