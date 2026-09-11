#pragma once

#include "app/Presets.h"
#include "engine/AudioEngine.h"
#include "gui/LevelMeter.h"
#include "gui/Oscilloscope.h"
#include "io/PresetStore.h"
#include "model/Preset.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <vector>

namespace noisefield::app
{

/// Root content component: transport, a collapsible oscilloscope, a level meter, a tone
/// source (frequency + level), a noise source (colour + level), a master level and a session
/// timer (duration + fade in/out, auto-stop). The soft limiter and audio-device settings, and
/// the user guide, live in their own detached windows. Control values (and the scope's
/// expanded state) persist between runs.
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

    void applyPreset(const model::Preset& preset);
    [[nodiscard]] model::Preset readState();
    void rebuildPresetMenu();
    void promptSavePreset();
    void deleteSelectedPreset();

    void openSettingsWindow();
    void openGuideWindow();
    void setScopeExpanded(bool expanded);

    engine::EngineParameters& params()
    {
        return engine_.parameters();
    }

    juce::ApplicationProperties appProperties_;
    engine::AudioEngine engine_;

    juce::TextButton playButton_{"Play"};
    juce::TextButton masterMuteButton_{"Mute"};
    juce::TextButton scopeButton_{"Scope"};
    juce::TextButton guideButton_{"Guide"};
    juce::TextButton settingsButton_{"Settings"};
    gui::Oscilloscope oscilloscope_;
    gui::LevelMeter meter_;

    juce::Label presetLabel_;
    juce::ComboBox presetBox_;
    juce::TextButton savePresetButton_{"Save"};
    juce::TextButton deletePresetButton_{"Delete"};
    io::PresetStore presetStore_;
    std::vector<juce::String> userPresetNames_;

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

    juce::Label sessionHeading_;
    juce::Slider sessionDurationSlider_;
    juce::Slider sessionFadeInSlider_;
    juce::Slider sessionFadeOutSlider_;
    juce::TextButton sessionStartButton_{"Start Timer"};
    juce::Label sessionStatusLabel_;

    juce::Label statusLabel_;

    bool scopeExpanded_ = false;

    bool sessionRunning_ = false;
    double sessionElapsedSeconds_ = 0.0;
    double sessionDurationSeconds_ = 0.0;
    double sessionFadeInSeconds_ = 0.0;
    double sessionFadeOutSeconds_ = 0.0;
    float sessionTargetGainDb_ = 0.0f;

    void startSessionTimer();
    void cancelSessionTimer();
    void tickSessionTimer();

    std::unique_ptr<juce::DocumentWindow> settingsWindow_;
    std::unique_ptr<juce::DocumentWindow> guideWindow_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace noisefield::app
