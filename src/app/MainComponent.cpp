#include "app/MainComponent.h"

#include "core/BuildInfo.h"
#include "dsp/Constants.h"
#include "gui/DetachedWindow.h"
#include "gui/GuideView.h"
#include "gui/SettingsComponent.h"

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
constexpr auto kMasterGainKey = "masterGainDb";
constexpr auto kLimiterKey = "limiterEnabled";
constexpr auto kAudioStateKey = "audioDeviceState";

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
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Noisefield";
    options.filenameSuffix = ".settings";
    options.folderName = "Noisefield";
    options.osxLibrarySubFolder = "Application Support";
    appProperties_.setStorageParameters(options);

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

    guideButton_.onClick = [this]
    {
        openGuideWindow();
    };
    settingsButton_.onClick = [this]
    {
        openSettingsWindow();
    };

    styleHeading(toneHeading_, "Tone");
    toneEnableButton_.onClick = [this]
    {
        params().toneEnabled.store(toneEnableButton_.getToggleState(), std::memory_order_relaxed);
    };

    frequencySlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    frequencySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 96, 22);
    frequencySlider_.setRange(dsp::kMinFrequencyHz, dsp::kMaxFrequencyHz, 0.0);
    frequencySlider_.setSkewFactorFromMidPoint(632.0);
    frequencySlider_.setTextValueSuffix(" Hz");
    frequencySlider_.setNumDecimalPlacesToDisplay(1);
    frequencySlider_.onValueChange = [this]
    {
        params().toneFrequencyHz.store(static_cast<float>(frequencySlider_.getValue()),
                                       std::memory_order_relaxed);
    };

    configureGainSlider(toneGainSlider_);
    toneGainSlider_.onValueChange = [this]
    {
        params().toneGainDb.store(static_cast<float>(toneGainSlider_.getValue()),
                                  std::memory_order_relaxed);
    };

    styleHeading(noiseHeading_, "White noise");
    noiseEnableButton_.onClick = [this]
    {
        params().noiseEnabled.store(noiseEnableButton_.getToggleState(), std::memory_order_relaxed);
    };

    configureGainSlider(noiseGainSlider_);
    noiseGainSlider_.onValueChange = [this]
    {
        params().noiseGainDb.store(static_cast<float>(noiseGainSlider_.getValue()),
                                   std::memory_order_relaxed);
    };

    reseedButton_.onClick = [this]
    {
        const auto seed =
            static_cast<std::uint64_t>(juce::Random::getSystemRandom().nextInt64()) | 1ULL;
        params().noiseSeed.store(seed, std::memory_order_relaxed);
    };

    styleHeading(masterHeading_, "Master");
    configureGainSlider(masterGainSlider_);
    masterGainSlider_.onValueChange = [this]
    {
        params().masterGainDb.store(static_cast<float>(masterGainSlider_.getValue()),
                                    std::memory_order_relaxed);
    };

    statusLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    statusLabel_.setJustificationType(juce::Justification::centredLeft);

    loadSettings();
    pushAllParametersToEngine();

    auto audioState = appProperties_.getUserSettings()->getXmlValue(kAudioStateKey);
    if (const auto error = engine_.initialise(audioState.get()); error.isNotEmpty())
        statusLabel_.setText("Audio error: " + error, juce::dontSendNotification);

    for (juce::Component* c : std::initializer_list<juce::Component*>{&playButton_,
                                                                      &masterMuteButton_,
                                                                      &guideButton_,
                                                                      &settingsButton_,
                                                                      &meter_,
                                                                      &toneHeading_,
                                                                      &toneEnableButton_,
                                                                      &frequencySlider_,
                                                                      &toneGainSlider_,
                                                                      &noiseHeading_,
                                                                      &noiseEnableButton_,
                                                                      &noiseGainSlider_,
                                                                      &reseedButton_,
                                                                      &masterHeading_,
                                                                      &masterGainSlider_,
                                                                      &statusLabel_})
        addAndMakeVisible(c);

    setSize(470, 470);
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

void MainComponent::openSettingsWindow()
{
    if (settingsWindow_ != nullptr)
    {
        settingsWindow_->toFront(true);
        return;
    }
    settingsWindow_ =
        std::make_unique<gui::DetachedWindow>("Noisefield — Settings",
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
    guideWindow_ = std::make_unique<gui::DetachedWindow>(
        "Noisefield — Guide", std::make_unique<gui::GuideView>(), [this] { guideWindow_.reset(); });
}

void MainComponent::loadSettings()
{
    auto* store = appProperties_.getUserSettings();

    frequencySlider_.setValue(store->getDoubleValue(kFreqKey, params().toneFrequencyHz.load()),
                              juce::dontSendNotification);
    toneGainSlider_.setValue(store->getDoubleValue(kToneGainKey, params().toneGainDb.load()),
                             juce::dontSendNotification);
    toneEnableButton_.setToggleState(
        store->getBoolValue(kToneEnabledKey, params().toneEnabled.load()),
        juce::dontSendNotification);
    noiseGainSlider_.setValue(store->getDoubleValue(kNoiseGainKey, params().noiseGainDb.load()),
                              juce::dontSendNotification);
    noiseEnableButton_.setToggleState(
        store->getBoolValue(kNoiseEnabledKey, params().noiseEnabled.load()),
        juce::dontSendNotification);
    masterGainSlider_.setValue(store->getDoubleValue(kMasterGainKey, params().masterGainDb.load()),
                               juce::dontSendNotification);

    params().limiterEnabled.store(store->getBoolValue(kLimiterKey, params().limiterEnabled.load()),
                                  std::memory_order_relaxed);
}

void MainComponent::saveSettings()
{
    auto* store = appProperties_.getUserSettings();

    store->setValue(kFreqKey, frequencySlider_.getValue());
    store->setValue(kToneGainKey, toneGainSlider_.getValue());
    store->setValue(kToneEnabledKey, toneEnableButton_.getToggleState());
    store->setValue(kNoiseGainKey, noiseGainSlider_.getValue());
    store->setValue(kNoiseEnabledKey, noiseEnableButton_.getToggleState());
    store->setValue(kMasterGainKey, masterGainSlider_.getValue());
    store->setValue(kLimiterKey, params().limiterEnabled.load(std::memory_order_relaxed));

    if (auto stateXml = engine_.deviceManager().createStateXml())
        store->setValue(kAudioStateKey, stateXml.get());

    store->saveIfNeeded();
}

void MainComponent::pushAllParametersToEngine()
{
    auto& p = params();
    p.toneFrequencyHz.store(static_cast<float>(frequencySlider_.getValue()),
                            std::memory_order_relaxed);
    p.toneGainDb.store(static_cast<float>(toneGainSlider_.getValue()), std::memory_order_relaxed);
    p.toneEnabled.store(toneEnableButton_.getToggleState(), std::memory_order_relaxed);
    p.noiseGainDb.store(static_cast<float>(noiseGainSlider_.getValue()), std::memory_order_relaxed);
    p.noiseEnabled.store(noiseEnableButton_.getToggleState(), std::memory_order_relaxed);
    p.masterGainDb.store(static_cast<float>(masterGainSlider_.getValue()),
                         std::memory_order_relaxed);
    // limiterEnabled is already populated from settings in loadSettings().
}

void MainComponent::timerCallback()
{
    const auto level = engine_.fetchMeterAndReset();
    meter_.setLevel(level.peak, level.rms);

    const auto rate = engine_.sampleRate();
    const auto xruns = engine_.xRunCount();
    juce::String status;
    status << noisefield::buildInfoString() << "   ";
    status << (rate > 0.0 ? juce::String(rate, 0) + " Hz" : juce::String("audio stopped"));
    status << "   xruns: " << (xruns < 0 ? juce::String("n/a") : juce::String(xruns));
    statusLabel_.setText(status, juce::dontSendNotification);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16);

    auto transport = area.removeFromTop(30);
    playButton_.setBounds(transport.removeFromLeft(96));
    transport.removeFromLeft(8);
    masterMuteButton_.setBounds(transport.removeFromLeft(96));
    settingsButton_.setBounds(transport.removeFromRight(96));
    transport.removeFromRight(8);
    guideButton_.setBounds(transport.removeFromRight(80));

    area.removeFromTop(10);
    meter_.setBounds(area.removeFromTop(16));
    area.removeFromTop(16);

    toneHeading_.setBounds(area.removeFromTop(20));
    auto toneRow = area.removeFromTop(120);
    frequencySlider_.setBounds(toneRow.removeFromLeft(150));
    toneRow.removeFromLeft(12);
    toneEnableButton_.setBounds(toneRow.removeFromTop(28));
    toneRow.removeFromTop(8);
    toneGainSlider_.setBounds(toneRow.removeFromTop(28));
    area.removeFromTop(12);

    noiseHeading_.setBounds(area.removeFromTop(20));
    noiseEnableButton_.setBounds(area.removeFromTop(28));
    area.removeFromTop(4);
    {
        auto row = area.removeFromTop(28);
        reseedButton_.setBounds(row.removeFromRight(90));
        row.removeFromRight(10);
        noiseGainSlider_.setBounds(row);
    }
    area.removeFromTop(12);

    masterHeading_.setBounds(area.removeFromTop(20));
    masterGainSlider_.setBounds(area.removeFromTop(28));

    statusLabel_.setBounds(area.removeFromBottom(22));
}

} // namespace noisefield::app
