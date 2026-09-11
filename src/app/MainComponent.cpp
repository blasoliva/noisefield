#include "app/MainComponent.h"

#include "dsp/Constants.h"
#include "dsp/NoiseColour.h"
#include "gui/DetachedWindow.h"
#include "gui/GuideView.h"
#include "gui/SettingsComponent.h"
#include "model/PresetJson.h"

#include <algorithm>
#include <initializer_list>
#include <memory>

namespace noisefield::app
{

namespace
{
constexpr auto kFreqKey = "toneFrequencyHz";
constexpr auto kToneGainKey = "toneGainDb";
constexpr auto kToneEnabledKey = "toneEnabled";
constexpr auto kNoiseGainKey = "noiseGainDb";
constexpr auto kNoiseEnabledKey = "noiseEnabled";
constexpr auto kNoiseColourKey = "noiseColour";
constexpr auto kMasterGainKey = "masterGainDb";
constexpr auto kLimiterKey = "limiterEnabled";
constexpr auto kScopeExpandedKey = "scopeExpanded";
constexpr auto kTimerExpandedKey = "timerExpanded";
constexpr auto kAudioStateKey = "audioDeviceState";
constexpr auto kSessionDurationKey = "sessionDurationMinutes";
constexpr auto kSessionFadeInKey = "sessionFadeInSeconds";
constexpr auto kSessionFadeOutKey = "sessionFadeOutSeconds";

// Card layout: consistent padding inside every gui::SectionCard, a gap between cards, and a
// smaller gap between a card's own rows (header -> first control, or control -> control).
constexpr int kOuterPad = 18;
constexpr int kCardPad = 16;
constexpr int kCardGap = 14;
constexpr int kRowGap = 8;
constexpr int kHeadGap = 10;

constexpr int kBaseHeight = 616;       // window height, scope + session timer both collapsed
constexpr int kScopeBlockHeight = 92;  // extra height when the scope is expanded
constexpr int kTimerBlockHeight = 178; // extra height when the session timer is expanded

int computeWindowHeight(bool scopeExpanded, bool timerExpanded)
{
    return kBaseHeight + (scopeExpanded ? kScopeBlockHeight : 0) +
           (timerExpanded ? kTimerBlockHeight : 0);
}

constexpr int kFactoryIdBase = 1; // ComboBox item ids for the factory presets
constexpr int kUserIdBase = 1000; // ... and the user presets

// Until the layer-list GUI (NF-042) lands, the main window drives two fixed engine layer
// slots: slot 0 is the tone oscillator, slot 1 is the noise source.
constexpr int kToneLayer = 0;
constexpr int kNoiseLayer = 1;

// juce::String(const char*) decodes bytes as ASCII and mangles anything non-ASCII, so route
// user-facing literals that contain non-ASCII characters (e.g. an em dash) through this.
juce::String uiString(const char* utf8)
{
    return juce::String::fromUTF8(utf8);
}

void styleHeading(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, juce::Colour(0xffb9c0c8));
}

void configureGainSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 22);
    slider.setRange(dsp::kMinGainDb, 0.0, 0.1);
    slider.setTextValueSuffix(" dB");
}
} // namespace

MainComponent::MainComponent()
    : oscilloscope_([this](float* dst, int count) { engine_.readScope(dst, count); })
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Noisefield";
    options.filenameSuffix = ".settings";
    // On Linux JUCE roots this at "~/<folderName>", so spell out the XDG config dir.
    options.folderName = ".config/Noisefield";
    options.osxLibrarySubFolder = "Application Support";
    appProperties_.setStorageParameters(options);

    const auto initLayer = [this](int slot, engine::LayerSource source)
    {
        auto& layer = params().layer(slot);
        layer.source.store(static_cast<int>(source), std::memory_order_relaxed);
        layer.active.store(true, std::memory_order_relaxed);
        layer.epoch.store(1, std::memory_order_relaxed);
    };
    initLayer(kToneLayer, engine::LayerSource::Oscillator);
    initLayer(kNoiseLayer, engine::LayerSource::Noise);

    playButton_.setClickingTogglesState(true);
    playButton_.onClick = [this]
    {
        const bool on = playButton_.getToggleState();
        params().playing.store(on, std::memory_order_relaxed);
        playButton_.setButtonText(on ? "Stop" : "Play");
    };

    masterMuteButton_.setClickingTogglesState(true);
    masterMuteButton_.onClick = [this]
    {
        const bool muted = masterMuteButton_.getToggleState();
        params().masterMute.store(muted, std::memory_order_relaxed);
        masterMuteButton_.setButtonText(muted ? "Muted" : "Mute");
    };

    scopeButton_.setTooltip("Oscilloscope");
    scopeButton_.setClickingTogglesState(true);
    scopeButton_.onClick = [this]
    {
        setScopeExpanded(scopeButton_.getToggleState());
    };
    timerButton_.setTooltip("Session timer");
    timerButton_.setClickingTogglesState(true);
    timerButton_.onClick = [this]
    {
        setTimerExpanded(timerButton_.getToggleState());
    };
    guideButton_.setTooltip("Guide");
    guideButton_.onClick = [this]
    {
        openGuideWindow();
    };
    settingsButton_.setTooltip("Settings");
    settingsButton_.onClick = [this]
    {
        openSettingsWindow();
    };

    presetLabel_.setText("Preset", juce::dontSendNotification);
    presetLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffb9c0c8));
    presetBox_.setTextWhenNothingSelected("Choose a preset");
    presetBox_.onChange = [this]
    {
        const int id = presetBox_.getSelectedId();
        const auto& factory = factoryPresets();
        if (id >= kFactoryIdBase && id < kFactoryIdBase + static_cast<int>(factory.size()))
        {
            applyPreset(factory[static_cast<size_t>(id - kFactoryIdBase)]);
            deletePresetButton_.setEnabled(false);
        }
        else if (id >= kUserIdBase && id < kUserIdBase + static_cast<int>(userPresetNames_.size()))
        {
            const auto name = userPresetNames_[static_cast<size_t>(id - kUserIdBase)];
            if (const auto preset = presetStore_.load(name))
                applyPreset(*preset);
            deletePresetButton_.setEnabled(true);
        }
    };

    savePresetButton_.onClick = [this]
    {
        promptSavePreset();
    };
    deletePresetButton_.onClick = [this]
    {
        deleteSelectedPreset();
    };
    deletePresetButton_.setEnabled(false);
    rebuildPresetMenu();

    styleHeading(toneHeading_, "Tone");
    toneEnableButton_.onClick = [this]
    {
        params()
            .layer(kToneLayer)
            .muted.store(!toneEnableButton_.getToggleState(), std::memory_order_relaxed);
    };

    frequencySlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    frequencySlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 22);
    frequencySlider_.setRange(dsp::kMinFrequencyHz, dsp::kMaxFrequencyHz, 0.0);
    frequencySlider_.setSkewFactorFromMidPoint(632.0);
    frequencySlider_.setTextValueSuffix(" Hz");
    frequencySlider_.setNumDecimalPlacesToDisplay(1);
    frequencySlider_.onValueChange = [this]
    {
        params()
            .layer(kToneLayer)
            .frequencyHz.store(static_cast<float>(frequencySlider_.getValue()),
                               std::memory_order_relaxed);
    };

    configureGainSlider(toneGainSlider_);
    toneGainSlider_.onValueChange = [this]
    {
        params()
            .layer(kToneLayer)
            .gainDb.store(static_cast<float>(toneGainSlider_.getValue()),
                          std::memory_order_relaxed);
    };

    styleHeading(noiseHeading_, "Noise");
    noiseEnableButton_.onClick = [this]
    {
        params()
            .layer(kNoiseLayer)
            .muted.store(!noiseEnableButton_.getToggleState(), std::memory_order_relaxed);
    };

    for (size_t i = 0; i < dsp::kNoiseColours.size(); ++i)
        noiseColourBox_.addItem(dsp::noiseColourName(dsp::kNoiseColours[i]),
                                static_cast<int>(i) + 1);
    noiseColourBox_.onChange = [this]
    {
        params()
            .layer(kNoiseLayer)
            .noiseColour.store(noiseColourBox_.getSelectedId() - 1, std::memory_order_relaxed);
    };

    configureGainSlider(noiseGainSlider_);
    noiseGainSlider_.onValueChange = [this]
    {
        params()
            .layer(kNoiseLayer)
            .gainDb.store(static_cast<float>(noiseGainSlider_.getValue()),
                          std::memory_order_relaxed);
    };

    reseedButton_.onClick = [this]
    {
        const auto seed =
            static_cast<std::uint64_t>(juce::Random::getSystemRandom().nextInt64()) | 1ULL;
        params().layer(kNoiseLayer).seed.store(seed, std::memory_order_relaxed);
    };

    styleHeading(masterHeading_, "Master");
    masterCard_.setHighlighted(true);
    configureGainSlider(masterGainSlider_);
    masterGainSlider_.onValueChange = [this]
    {
        params().masterGainDb.store(static_cast<float>(masterGainSlider_.getValue()),
                                    std::memory_order_relaxed);
    };

    styleHeading(sessionHeading_, "Session timer");

    sessionDurationSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    sessionDurationSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 22);
    sessionDurationSlider_.setRange(1.0, 180.0, 1.0);
    sessionDurationSlider_.setTextValueSuffix(" min");

    sessionFadeInSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    sessionFadeInSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 55, 22);
    sessionFadeInSlider_.setRange(0.0, 60.0, 1.0);
    sessionFadeInSlider_.setTextValueSuffix(" s in");

    sessionFadeOutSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    sessionFadeOutSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 55, 22);
    sessionFadeOutSlider_.setRange(0.0, 60.0, 1.0);
    sessionFadeOutSlider_.setTextValueSuffix(" s out");

    sessionStartButton_.setClickingTogglesState(true);
    sessionStartButton_.onClick = [this]
    {
        if (sessionStartButton_.getToggleState())
            startSessionTimer();
        else
            cancelSessionTimer();
    };

    sessionStatusLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    sessionStatusLabel_.setText("Timer off", juce::dontSendNotification);

    statusLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    statusLabel_.setJustificationType(juce::Justification::centredLeft);

    for (auto* label : {&rateLabel_, &peakLabel_, &xrunsLabel_})
        label->setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    rateLabel_.setJustificationType(juce::Justification::centredLeft);
    peakLabel_.setJustificationType(juce::Justification::centred);
    xrunsLabel_.setJustificationType(juce::Justification::centredRight);

    loadSettings();
    pushAllParametersToEngine();

    auto audioState = appProperties_.getUserSettings()->getXmlValue(kAudioStateKey);
    if (const auto error = engine_.initialise(audioState.get()); error.isNotEmpty())
        statusLabel_.setText("Audio error: " + error, juce::dontSendNotification);

    for (juce::Component* c : std::initializer_list<juce::Component*>{
             &playButton_,
             &masterMuteButton_,
             &scopeButton_,
             &timerButton_,
             &guideButton_,
             &settingsButton_,
             &monitorCard_,
             &oscilloscope_,
             &meter_,
             &statusLabel_,
             &rateLabel_,
             &peakLabel_,
             &xrunsLabel_,
             &presetCard_,
             &presetLabel_,
             &presetBox_,
             &savePresetButton_,
             &deletePresetButton_,
             &toneCard_,
             &toneHeading_,
             &toneEnableButton_,
             &frequencySlider_,
             &toneGainSlider_,
             &noiseCard_,
             &noiseHeading_,
             &noiseEnableButton_,
             &noiseColourBox_,
             &noiseGainSlider_,
             &reseedButton_,
             &masterCard_,
             &masterHeading_,
             &masterGainSlider_,
             &sessionCard_,
             &sessionHeading_,
             &sessionDurationSlider_,
             &sessionFadeInSlider_,
             &sessionFadeOutSlider_,
             &sessionStartButton_,
             &sessionStatusLabel_})
        addAndMakeVisible(c);

    oscilloscope_.setVisible(scopeExpanded_);
    scopeButton_.setToggleState(scopeExpanded_, juce::dontSendNotification);

    timerButton_.setToggleState(timerExpanded_, juce::dontSendNotification);
    for (juce::Component* c : std::initializer_list<juce::Component*>{&sessionCard_,
                                                                      &sessionHeading_,
                                                                      &sessionDurationSlider_,
                                                                      &sessionFadeInSlider_,
                                                                      &sessionFadeOutSlider_,
                                                                      &sessionStartButton_,
                                                                      &sessionStatusLabel_})
        c->setVisible(timerExpanded_);

    setSize(480, computeWindowHeight(scopeExpanded_, timerExpanded_));
    startTimerHz(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    settingsWindow_.reset();
    guideWindow_.reset();
    saveSettings();
    engine_.shutdown();
}

void MainComponent::setScopeExpanded(bool expanded)
{
    scopeExpanded_ = expanded;
    oscilloscope_.setVisible(expanded);
    scopeButton_.setToggleState(expanded, juce::dontSendNotification);

    const int target = computeWindowHeight(scopeExpanded_, timerExpanded_);
    if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
        window->setContentComponentSize(getWidth(), target);
    else
        setSize(getWidth(), target);
}

void MainComponent::setTimerExpanded(bool expanded)
{
    timerExpanded_ = expanded;
    timerButton_.setToggleState(expanded, juce::dontSendNotification);
    for (juce::Component* c : std::initializer_list<juce::Component*>{&sessionCard_,
                                                                      &sessionHeading_,
                                                                      &sessionDurationSlider_,
                                                                      &sessionFadeInSlider_,
                                                                      &sessionFadeOutSlider_,
                                                                      &sessionStartButton_,
                                                                      &sessionStatusLabel_})
        c->setVisible(expanded);

    const int target = computeWindowHeight(scopeExpanded_, timerExpanded_);
    if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
        window->setContentComponentSize(getWidth(), target);
    else
        setSize(getWidth(), target);
}

void MainComponent::openSettingsWindow()
{
    if (settingsWindow_ != nullptr)
    {
        settingsWindow_->toFront(true);
        return;
    }
    settingsWindow_ =
        std::make_unique<gui::DetachedWindow>(uiString("Noisefield — Settings"),
                                              std::make_unique<gui::SettingsComponent>(engine_),
                                              [this] { settingsWindow_.reset(); });
}

void MainComponent::openGuideWindow()
{
    if (guideWindow_ != nullptr)
    {
        guideWindow_->toFront(true);
        return;
    }
    guideWindow_ = std::make_unique<gui::DetachedWindow>(uiString("Noisefield — Guide"),
                                                         std::make_unique<gui::GuideView>(),
                                                         [this] { guideWindow_.reset(); });
}

void MainComponent::loadSettings()
{
    auto* store = appProperties_.getUserSettings();

    frequencySlider_.setValue(store->getDoubleValue(kFreqKey, 220.0), juce::dontSendNotification);
    toneGainSlider_.setValue(store->getDoubleValue(kToneGainKey, -14.0),
                             juce::dontSendNotification);
    toneEnableButton_.setToggleState(store->getBoolValue(kToneEnabledKey, true),
                                     juce::dontSendNotification);
    noiseGainSlider_.setValue(store->getDoubleValue(kNoiseGainKey, -20.0),
                              juce::dontSendNotification);
    noiseEnableButton_.setToggleState(store->getBoolValue(kNoiseEnabledKey, false),
                                      juce::dontSendNotification);
    noiseColourBox_.setSelectedId(store->getIntValue(kNoiseColourKey, 0) + 1,
                                  juce::dontSendNotification);
    masterGainSlider_.setValue(store->getDoubleValue(kMasterGainKey, params().masterGainDb.load()),
                               juce::dontSendNotification);

    params().limiterEnabled.store(store->getBoolValue(kLimiterKey, params().limiterEnabled.load()),
                                  std::memory_order_relaxed);

    scopeExpanded_ = store->getBoolValue(kScopeExpandedKey, false);
    timerExpanded_ = store->getBoolValue(kTimerExpandedKey, false);

    sessionDurationSlider_.setValue(store->getDoubleValue(kSessionDurationKey, 30.0),
                                    juce::dontSendNotification);
    sessionFadeInSlider_.setValue(store->getDoubleValue(kSessionFadeInKey, 5.0),
                                  juce::dontSendNotification);
    sessionFadeOutSlider_.setValue(store->getDoubleValue(kSessionFadeOutKey, 10.0),
                                   juce::dontSendNotification);
}

void MainComponent::saveSettings()
{
    auto* store = appProperties_.getUserSettings();

    store->setValue(kFreqKey, frequencySlider_.getValue());
    store->setValue(kToneGainKey, toneGainSlider_.getValue());
    store->setValue(kToneEnabledKey, toneEnableButton_.getToggleState());
    store->setValue(kNoiseGainKey, noiseGainSlider_.getValue());
    store->setValue(kNoiseEnabledKey, noiseEnableButton_.getToggleState());
    store->setValue(kNoiseColourKey, noiseColourBox_.getSelectedId() - 1);
    store->setValue(kMasterGainKey, masterGainSlider_.getValue());
    store->setValue(kLimiterKey, params().limiterEnabled.load(std::memory_order_relaxed));
    store->setValue(kScopeExpandedKey, scopeExpanded_);
    store->setValue(kTimerExpandedKey, timerExpanded_);

    store->setValue(kSessionDurationKey, sessionDurationSlider_.getValue());
    store->setValue(kSessionFadeInKey, sessionFadeInSlider_.getValue());
    store->setValue(kSessionFadeOutKey, sessionFadeOutSlider_.getValue());

    if (auto stateXml = engine_.deviceManager().createStateXml())
        store->setValue(kAudioStateKey, stateXml.get());

    store->saveIfNeeded();
}

void MainComponent::applyPreset(const model::Preset& preset)
{
    int colourIndex = 0;
    for (size_t i = 0; i < dsp::kNoiseColours.size(); ++i)
        if (dsp::kNoiseColours[i] == preset.noiseColour)
            colourIndex = static_cast<int>(i);

    toneEnableButton_.setToggleState(preset.toneEnabled, juce::sendNotification);
    frequencySlider_.setValue(preset.toneFrequencyHz, juce::sendNotification);
    toneGainSlider_.setValue(preset.toneGainDb, juce::sendNotification);
    noiseEnableButton_.setToggleState(preset.noiseEnabled, juce::sendNotification);
    noiseColourBox_.setSelectedId(colourIndex + 1, juce::sendNotification);
    noiseGainSlider_.setValue(preset.noiseGainDb, juce::sendNotification);
    masterMuteButton_.setToggleState(preset.masterMute, juce::sendNotification);
    masterGainSlider_.setValue(preset.masterGainDb, juce::sendNotification);
    params().limiterEnabled.store(preset.limiterEnabled, std::memory_order_relaxed);
}

model::Preset MainComponent::readState()
{
    model::Preset preset;
    preset.toneEnabled = toneEnableButton_.getToggleState();
    preset.toneFrequencyHz = frequencySlider_.getValue();
    preset.toneGainDb = toneGainSlider_.getValue();
    preset.noiseEnabled = noiseEnableButton_.getToggleState();
    preset.noiseColour = dsp::kNoiseColours[static_cast<size_t>(juce::jlimit(
        0, static_cast<int>(dsp::kNoiseColours.size()) - 1, noiseColourBox_.getSelectedId() - 1))];
    preset.noiseGainDb = noiseGainSlider_.getValue();
    preset.noiseSeed = params().layer(kNoiseLayer).seed.load(std::memory_order_relaxed);
    preset.masterMute = masterMuteButton_.getToggleState();
    preset.masterGainDb = masterGainSlider_.getValue();
    preset.limiterEnabled = params().limiterEnabled.load(std::memory_order_relaxed);
    return preset;
}

void MainComponent::rebuildPresetMenu()
{
    const auto previous = presetBox_.getText();
    userPresetNames_ = presetStore_.list();

    presetBox_.clear(juce::dontSendNotification);
    presetBox_.addSectionHeading("Factory");
    const auto& factory = factoryPresets();
    for (size_t i = 0; i < factory.size(); ++i)
        presetBox_.addItem(factory[i].name, kFactoryIdBase + static_cast<int>(i));

    if (!userPresetNames_.empty())
    {
        presetBox_.addSeparator();
        presetBox_.addSectionHeading("User");
        for (size_t i = 0; i < userPresetNames_.size(); ++i)
            presetBox_.addItem(userPresetNames_[i], kUserIdBase + static_cast<int>(i));
    }

    if (previous.isNotEmpty())
        for (int i = 0; i < presetBox_.getNumItems(); ++i)
            if (presetBox_.getItemText(i) == previous)
                presetBox_.setSelectedItemIndex(i, juce::dontSendNotification);
}

void MainComponent::promptSavePreset()
{
    auto* prompt = new juce::AlertWindow(
        "Save preset", "Name for this preset:", juce::MessageBoxIconType::NoIcon);
    prompt->addTextEditor("name",
                          presetBox_.getText().isNotEmpty() ? presetBox_.getText()
                                                            : juce::String("My preset"));
    prompt->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    prompt->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    juce::Component::SafePointer<MainComponent> self(this);
    prompt->enterModalState(true,
                            juce::ModalCallbackFunction::create(
                                [self, prompt](int result)
                                {
                                    const juce::String name =
                                        prompt->getTextEditorContents("name").trim();
                                    if (self == nullptr || result != 1 || name.isEmpty())
                                        return;

                                    auto preset = self->readState();
                                    preset.name = name.toStdString();
                                    if (self->presetStore_.save(preset))
                                    {
                                        self->rebuildPresetMenu();
                                        self->presetBox_.setText(name, juce::dontSendNotification);
                                        self->deletePresetButton_.setEnabled(true);
                                    }
                                }),
                            true);
}

void MainComponent::deleteSelectedPreset()
{
    const int id = presetBox_.getSelectedId();
    if (id < kUserIdBase || id >= kUserIdBase + static_cast<int>(userPresetNames_.size()))
        return;
    const auto name = userPresetNames_[static_cast<size_t>(id - kUserIdBase)];

    juce::Component::SafePointer<MainComponent> self(this);
    juce::AlertWindow::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Delete preset",
        "Delete \"" + name + "\"?",
        "Delete",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [self, name](int result)
            {
                if (self != nullptr && result == 1 && self->presetStore_.remove(name))
                {
                    self->rebuildPresetMenu();
                    self->presetBox_.setSelectedId(0, juce::dontSendNotification);
                    self->deletePresetButton_.setEnabled(false);
                }
            }));
}

void MainComponent::pushAllParametersToEngine()
{
    auto& p = params();
    auto& tone = p.layer(kToneLayer);
    auto& noise = p.layer(kNoiseLayer);

    tone.frequencyHz.store(static_cast<float>(frequencySlider_.getValue()),
                           std::memory_order_relaxed);
    tone.gainDb.store(static_cast<float>(toneGainSlider_.getValue()), std::memory_order_relaxed);
    tone.muted.store(!toneEnableButton_.getToggleState(), std::memory_order_relaxed);

    noise.gainDb.store(static_cast<float>(noiseGainSlider_.getValue()), std::memory_order_relaxed);
    noise.muted.store(!noiseEnableButton_.getToggleState(), std::memory_order_relaxed);
    noise.noiseColour.store(noiseColourBox_.getSelectedId() - 1, std::memory_order_relaxed);

    p.masterGainDb.store(static_cast<float>(masterGainSlider_.getValue()),
                         std::memory_order_relaxed);
    // limiterEnabled is already populated from settings in loadSettings().
}

void MainComponent::startSessionTimer()
{
    sessionDurationSeconds_ = sessionDurationSlider_.getValue() * 60.0;
    sessionFadeInSeconds_ = sessionFadeInSlider_.getValue();
    sessionFadeOutSeconds_ = sessionFadeOutSlider_.getValue();
    sessionTargetGainDb_ = static_cast<float>(masterGainSlider_.getValue());
    sessionElapsedSeconds_ = 0.0;
    sessionRunning_ = true;

    sessionStartButton_.setButtonText("Cancel Timer");
    sessionDurationSlider_.setEnabled(false);
    sessionFadeInSlider_.setEnabled(false);
    sessionFadeOutSlider_.setEnabled(false);

    if (!playButton_.getToggleState())
        playButton_.setToggleState(true, juce::sendNotification);

    if (sessionFadeInSeconds_ > 0.0)
        masterGainSlider_.setValue(dsp::kMinGainDb, juce::sendNotification);
}

void MainComponent::cancelSessionTimer()
{
    sessionRunning_ = false;
    sessionStartButton_.setToggleState(false, juce::dontSendNotification);
    sessionStartButton_.setButtonText("Start Timer");
    sessionDurationSlider_.setEnabled(true);
    sessionFadeInSlider_.setEnabled(true);
    sessionFadeOutSlider_.setEnabled(true);
    sessionStatusLabel_.setText("Timer off", juce::dontSendNotification);
    masterGainSlider_.setValue(sessionTargetGainDb_, juce::sendNotification);
}

void MainComponent::tickSessionTimer()
{
    if (!sessionRunning_)
        return;

    sessionElapsedSeconds_ += 1.0 / 30.0;

    if (sessionElapsedSeconds_ >= sessionDurationSeconds_)
    {
        sessionRunning_ = false;
        sessionStartButton_.setToggleState(false, juce::dontSendNotification);
        sessionStartButton_.setButtonText("Start Timer");
        sessionDurationSlider_.setEnabled(true);
        sessionFadeInSlider_.setEnabled(true);
        sessionFadeOutSlider_.setEnabled(true);
        playButton_.setToggleState(false, juce::sendNotification);
        masterGainSlider_.setValue(sessionTargetGainDb_, juce::sendNotification);
        sessionStatusLabel_.setText("Timer finished", juce::dontSendNotification);
        return;
    }

    const double fadeOutStart = std::max(0.0, sessionDurationSeconds_ - sessionFadeOutSeconds_);
    float gainDb = sessionTargetGainDb_;

    if (sessionElapsedSeconds_ < sessionFadeInSeconds_)
    {
        const double t = sessionElapsedSeconds_ / std::max(sessionFadeInSeconds_, 0.001);
        gainDb = dsp::kMinGainDb + static_cast<float>(t) * (sessionTargetGainDb_ - dsp::kMinGainDb);
    }
    else if (sessionElapsedSeconds_ >= fadeOutStart && sessionFadeOutSeconds_ > 0.0)
    {
        const double remain = std::max(0.0, sessionDurationSeconds_ - sessionElapsedSeconds_);
        const double t = juce::jlimit(0.0, 1.0, remain / sessionFadeOutSeconds_);
        gainDb = dsp::kMinGainDb + static_cast<float>(t) * (sessionTargetGainDb_ - dsp::kMinGainDb);
    }

    masterGainSlider_.setValue(gainDb, juce::sendNotification);

    const int remainingSeconds = static_cast<int>(sessionDurationSeconds_ - sessionElapsedSeconds_);
    sessionStatusLabel_.setText(
        juce::String::formatted("%d:%02d left", remainingSeconds / 60, remainingSeconds % 60),
        juce::dontSendNotification);
}

void MainComponent::timerCallback()
{
    const auto level = engine_.fetchMeterAndReset();
    meter_.setLevel(level.peak, level.rms);

    tickSessionTimer();

    // NF-073: the audio device (typically JACK) can disappear and come back at any time; try
    // to reopen it every ~3s rather than requiring the user to restart the app.
    if (engine_.deviceLost())
    {
        if (reconnectCooldown_ <= 0)
        {
            engine_.attemptReconnect();
            reconnectCooldown_ = 90; // ~3 s at 30 Hz
        }
        else
        {
            --reconnectCooldown_;
        }
    }
    else
    {
        reconnectCooldown_ = 0;
    }

    const auto rate = engine_.sampleRate();
    const auto xruns = engine_.xRunCount();
    const float peakDb = meter_.currentPeakDb();
    const bool lost = engine_.deviceLost();

    statusLabel_.setVisible(lost);
    for (auto* label : {&rateLabel_, &peakLabel_, &xrunsLabel_})
        label->setVisible(!lost);

    if (lost)
    {
        statusLabel_.setText("audio device lost, reconnecting" + uiString("…"),
                             juce::dontSendNotification);
        return;
    }

    rateLabel_.setText(rate > 0.0 ? juce::String(rate, 0) + " Hz" : juce::String("audio stopped"),
                       juce::dontSendNotification);
    peakLabel_.setText(
        "peak " + (peakDb <= -60.0f ? juce::String("-inf") : juce::String(peakDb, 1)) + " dBFS",
        juce::dontSendNotification);
    xrunsLabel_.setText("xruns: " + (xruns < 0 ? juce::String("n/a") : juce::String(xruns)),
                        juce::dontSendNotification);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

namespace
{
/// Carves a card's outer rect off `area` (with the gap already applied before it), sets
/// `card`'s bounds to it, and returns the padded inner rect the caller lays its own controls
/// into.
juce::Rectangle<int> takeCard(juce::Rectangle<int>& area, juce::Component& card, int innerHeight)
{
    auto outer = area.removeFromTop(innerHeight + kCardPad * 2);
    card.setBounds(outer);
    return outer.reduced(kCardPad);
}

/// Splits a card's header row into the heading label (left) and its on/off switch (right),
/// matching the switch size the LookAndFeel draws.
void layoutCardHeaderWithSwitch(juce::Rectangle<int> row,
                                juce::Label& heading,
                                juce::ToggleButton& switchButton)
{
    switchButton.setBounds(row.removeFromRight(40).withSizeKeepingCentre(38, 20));
    heading.setBounds(row);
}
} // namespace

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(kOuterPad);

    auto transport = area.removeFromTop(30);
    playButton_.setBounds(transport.removeFromLeft(80));
    transport.removeFromLeft(8);
    masterMuteButton_.setBounds(transport.removeFromLeft(80));
    settingsButton_.setBounds(transport.removeFromRight(40));
    transport.removeFromRight(6);
    guideButton_.setBounds(transport.removeFromRight(40));
    transport.removeFromRight(6);
    timerButton_.setBounds(transport.removeFromRight(40));
    transport.removeFromRight(6);
    scopeButton_.setBounds(transport.removeFromRight(40));
    area.removeFromTop(kCardGap);

    // ---- Monitor: oscilloscope (optional) + level meter + device status, one card ----
    {
        const int scopeBlock = scopeExpanded_ ? 84 + kRowGap : 0;
        auto inner = takeCard(area, monitorCard_, scopeBlock + 16 + kRowGap + 18);
        if (scopeExpanded_)
        {
            oscilloscope_.setBounds(inner.removeFromTop(84));
            inner.removeFromTop(kRowGap);
        }
        meter_.setBounds(inner.removeFromTop(16));
        inner.removeFromTop(kRowGap);
        auto statusRow = inner.removeFromTop(18);
        statusLabel_.setBounds(statusRow);
        const int third = statusRow.getWidth() / 3;
        rateLabel_.setBounds(statusRow.removeFromLeft(third));
        xrunsLabel_.setBounds(statusRow.removeFromRight(third));
        peakLabel_.setBounds(statusRow);
    }
    area.removeFromTop(kCardGap);

    // ---- Preset ----
    {
        auto inner = takeCard(area, presetCard_, 26);
        presetLabel_.setBounds(inner.removeFromLeft(46));
        deletePresetButton_.setBounds(inner.removeFromRight(60));
        inner.removeFromRight(6);
        savePresetButton_.setBounds(inner.removeFromRight(56));
        inner.removeFromRight(6);
        presetBox_.setBounds(inner);
    }
    area.removeFromTop(kCardGap);

    // ---- Tone ----
    {
        auto inner = takeCard(area, toneCard_, 22 + kHeadGap + 28 + kRowGap + 28);
        layoutCardHeaderWithSwitch(inner.removeFromTop(22), toneHeading_, toneEnableButton_);
        inner.removeFromTop(kHeadGap);
        frequencySlider_.setBounds(inner.removeFromTop(28));
        inner.removeFromTop(kRowGap);
        toneGainSlider_.setBounds(inner.removeFromTop(28));
    }
    area.removeFromTop(kCardGap);

    // ---- Noise ----
    {
        auto inner = takeCard(area, noiseCard_, 22 + kHeadGap + 28 + kRowGap + 28);
        layoutCardHeaderWithSwitch(inner.removeFromTop(22), noiseHeading_, noiseEnableButton_);
        inner.removeFromTop(kHeadGap);
        noiseColourBox_.setBounds(inner.removeFromTop(28));
        inner.removeFromTop(kRowGap);
        {
            auto row = inner.removeFromTop(28);
            reseedButton_.setBounds(row.removeFromRight(90));
            row.removeFromRight(10);
            noiseGainSlider_.setBounds(row);
        }
    }
    area.removeFromTop(kCardGap);

    // ---- Master ----
    {
        auto inner = takeCard(area, masterCard_, 22 + kHeadGap + 28);
        masterHeading_.setBounds(inner.removeFromTop(22));
        inner.removeFromTop(kHeadGap);
        masterGainSlider_.setBounds(inner.removeFromTop(28));
    }

    // ---- Session timer (collapsible, via the Timer button) ----
    if (timerExpanded_)
    {
        area.removeFromTop(kCardGap);
        auto inner =
            takeCard(area, sessionCard_, 22 + kHeadGap + 28 + kRowGap + 28 + kRowGap + 28);
        sessionHeading_.setBounds(inner.removeFromTop(22));
        inner.removeFromTop(kHeadGap);
        sessionDurationSlider_.setBounds(inner.removeFromTop(28));
        inner.removeFromTop(kRowGap);
        {
            auto row = inner.removeFromTop(28);
            sessionFadeInSlider_.setBounds(row.removeFromLeft(row.getWidth() / 2 - 5));
            row.removeFromLeft(10);
            sessionFadeOutSlider_.setBounds(row);
        }
        inner.removeFromTop(kRowGap);
        {
            auto row = inner.removeFromTop(28);
            sessionStartButton_.setBounds(row.removeFromLeft(110));
            row.removeFromLeft(10);
            sessionStatusLabel_.setBounds(row);
        }
    }
}

} // namespace noisefield::app
