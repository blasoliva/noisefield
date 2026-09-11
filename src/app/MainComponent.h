#pragma once

#include "app/Presets.h"
#include "engine/AudioEngine.h"
#include "gui/LevelMeter.h"
#include "gui/Oscilloscope.h"
#include "gui/SectionCard.h"
#include "io/PresetStore.h"
#include "model/Preset.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <vector>

namespace noisefield::app
{

/// Root content component: a transport bar over a stack of `gui::SectionCard`-backed panels
/// -- a Monitor (collapsible oscilloscope + level meter + device status), a Preset picker, a
/// Tone source (frequency + level), a Noise source (colour + level), a Master level, and a
/// collapsible session timer (duration + fade in/out, auto-stop). The soft limiter and
/// audio-device settings, and the user guide, live in their own detached windows. Control
/// values (and which cards are expanded) persist between runs.
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
    void setTimerExpanded(bool expanded);

    engine::EngineParameters& params()
    {
        return engine_.parameters();
    }

    juce::ApplicationProperties appProperties_;
    engine::AudioEngine engine_;

    juce::TextButton playButton_{"Play"};
    juce::TextButton masterMuteButton_{"Mute"};
    juce::TextButton scopeButton_{"Scope"};
    juce::TextButton timerButton_{"Timer"};
    juce::TextButton guideButton_{"Guide"};
    juce::TextButton settingsButton_{"Settings"};

    gui::SectionCard monitorCard_;
    gui::Oscilloscope oscilloscope_;
    gui::LevelMeter meter_;
    juce::Label rateLabel_;   // "44100 Hz", left
    juce::Label peakLabel_;   // "peak -x.x dBFS", centred
    juce::Label xrunsLabel_;  // "xruns: n", right

    gui::SectionCard presetCard_;
    juce::Label presetLabel_;
    juce::ComboBox presetBox_;
    juce::TextButton savePresetButton_{"Save"};
    juce::TextButton deletePresetButton_{"Delete"};
    io::PresetStore presetStore_;
    std::vector<juce::String> userPresetNames_;

    gui::SectionCard toneCard_;
    juce::Label toneHeading_;
    juce::ToggleButton toneEnableButton_;
    juce::Slider frequencySlider_;
    juce::Slider toneGainSlider_;

    gui::SectionCard noiseCard_;
    juce::Label noiseHeading_;
    juce::ToggleButton noiseEnableButton_;
    juce::ComboBox noiseColourBox_;
    juce::Slider noiseGainSlider_;
    juce::TextButton reseedButton_{"Re-seed"};

    gui::SectionCard masterCard_;
    juce::Label masterHeading_;
    juce::Slider masterGainSlider_;

    gui::SectionCard sessionCard_;
    juce::Label sessionHeading_;
    juce::Slider sessionDurationSlider_;
    juce::Slider sessionFadeInSlider_;
    juce::Slider sessionFadeOutSlider_;
    juce::TextButton sessionStartButton_{"Start Timer"};
    juce::Label sessionStatusLabel_;

    juce::Label statusLabel_;

    bool scopeExpanded_ = false;
    bool timerExpanded_ = false;

    bool sessionRunning_ = false;
    double sessionElapsedSeconds_ = 0.0;
    double sessionDurationSeconds_ = 0.0;
    double sessionFadeInSeconds_ = 0.0;
    double sessionFadeOutSeconds_ = 0.0;
    float sessionTargetGainDb_ = 0.0f;

    void startSessionTimer();
    void cancelSessionTimer();
    void tickSessionTimer();

    int reconnectCooldown_ = 0; // NF-073: ticks left before the next reconnect attempt

    std::unique_ptr<juce::DocumentWindow> settingsWindow_;
    std::unique_ptr<juce::DocumentWindow> guideWindow_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace noisefield::app
