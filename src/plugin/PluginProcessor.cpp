#include "plugin/PluginProcessor.h"

#include "dsp/Constants.h"
#include "dsp/NoiseColour.h"

#include <memory>

namespace noisefield::plugin
{

namespace
{
juce::StringArray noiseColourChoices()
{
    juce::StringArray names;
    for (const auto colour : dsp::kNoiseColours)
        names.add(dsp::noiseColourName(colour));
    return names;
}

juce::NormalisableRange<float> gainDbRange()
{
    return {dsp::kMinGainDb, 0.0f, 0.1f};
}

// The plugin drives two fixed engine layer slots (the dynamic layer list is a standalone-app
// feature for now): slot 0 is the tone oscillator, slot 1 is the noise source.
constexpr int kToneLayer = 0;
constexpr int kNoiseLayer = 1;
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout NoisefieldAudioProcessor::makeParameterLayout()
{
    using AF = juce::AudioParameterFloat;
    using AB = juce::AudioParameterBool;
    using AC = juce::AudioParameterChoice;

    juce::NormalisableRange<float> freqRange{static_cast<float>(dsp::kMinFrequencyHz),
                                             static_cast<float>(dsp::kMaxFrequencyHz)};
    freqRange.setSkewForCentre(632.0f);

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<AF>(
        juce::ParameterID{"masterGainDb", 1}, "Master level", gainDbRange(), -6.0f));
    layout.add(std::make_unique<AB>(juce::ParameterID{"masterMute", 1}, "Master mute", false));
    layout.add(std::make_unique<AB>(juce::ParameterID{"limiter", 1}, "Soft limiter", true));
    layout.add(std::make_unique<AB>(juce::ParameterID{"toneEnabled", 1}, "Tone enabled", true));
    layout.add(std::make_unique<AF>(
        juce::ParameterID{"toneFrequencyHz", 1}, "Tone frequency", freqRange, 220.0f));
    layout.add(std::make_unique<AF>(
        juce::ParameterID{"toneGainDb", 1}, "Tone level", gainDbRange(), -14.0f));
    layout.add(std::make_unique<AB>(juce::ParameterID{"noiseEnabled", 1}, "Noise enabled", false));
    layout.add(std::make_unique<AC>(
        juce::ParameterID{"noiseColour", 1}, "Noise colour", noiseColourChoices(), 0));
    layout.add(std::make_unique<AF>(
        juce::ParameterID{"noiseGainDb", 1}, "Noise level", gainDbRange(), -20.0f));
    return layout;
}

NoisefieldAudioProcessor::NoisefieldAudioProcessor()
    : juce::AudioProcessor(
          BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "PARAMETERS", makeParameterLayout())
{
    masterGainDb_ = apvts_.getRawParameterValue("masterGainDb");
    masterMute_ = apvts_.getRawParameterValue("masterMute");
    limiter_ = apvts_.getRawParameterValue("limiter");
    toneEnabled_ = apvts_.getRawParameterValue("toneEnabled");
    toneFrequencyHz_ = apvts_.getRawParameterValue("toneFrequencyHz");
    toneGainDb_ = apvts_.getRawParameterValue("toneGainDb");
    noiseEnabled_ = apvts_.getRawParameterValue("noiseEnabled");
    noiseColour_ = apvts_.getRawParameterValue("noiseColour");
    noiseGainDb_ = apvts_.getRawParameterValue("noiseGainDb");

    auto& p = graph_.parameters();
    p.layer(kToneLayer)
        .source.store(static_cast<int>(engine::LayerSource::Oscillator), std::memory_order_relaxed);
    p.layer(kToneLayer).active.store(true, std::memory_order_relaxed);
    p.layer(kToneLayer).epoch.store(1, std::memory_order_relaxed);
    p.layer(kNoiseLayer)
        .source.store(static_cast<int>(engine::LayerSource::Noise), std::memory_order_relaxed);
    p.layer(kNoiseLayer).active.store(true, std::memory_order_relaxed);
    p.layer(kNoiseLayer).epoch.store(1, std::memory_order_relaxed);

    // A plugin has no transport of its own -- it always produces sound while the host runs it.
    p.playing.store(true, std::memory_order_relaxed);
}

void NoisefieldAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    graph_.parameters().playing.store(true, std::memory_order_relaxed);
    graph_.prepare(sampleRate, samplesPerBlock);
}

bool NoisefieldAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (!layouts.getMainInputChannelSet().isDisabled())
        return false;

    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void NoisefieldAudioProcessor::syncParametersToGraph() noexcept
{
    auto& p = graph_.parameters();
    p.masterGainDb.store(masterGainDb_->load(), std::memory_order_relaxed);
    p.masterMute.store(masterMute_->load() > 0.5f, std::memory_order_relaxed);
    p.limiterEnabled.store(limiter_->load() > 0.5f, std::memory_order_relaxed);

    auto& tone = p.layer(kToneLayer);
    tone.muted.store(toneEnabled_->load() <= 0.5f, std::memory_order_relaxed);
    tone.frequencyHz.store(toneFrequencyHz_->load(), std::memory_order_relaxed);
    tone.gainDb.store(toneGainDb_->load(), std::memory_order_relaxed);

    auto& noise = p.layer(kNoiseLayer);
    noise.muted.store(noiseEnabled_->load() <= 0.5f, std::memory_order_relaxed);
    noise.noiseColour.store(static_cast<int>(noiseColour_->load() + 0.5f),
                            std::memory_order_relaxed);
    noise.gainDb.store(noiseGainDb_->load(), std::memory_order_relaxed);
}

void NoisefieldAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    syncParametersToGraph();
    graph_.process(
        buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples());
}

void NoisefieldAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts_.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void NoisefieldAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts_.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* NoisefieldAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

} // namespace noisefield::plugin

// NOLINTNEXTLINE(readability-identifier-naming) -- JUCE plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new noisefield::plugin::NoisefieldAudioProcessor();
}
