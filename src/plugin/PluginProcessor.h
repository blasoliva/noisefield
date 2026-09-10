#pragma once

#include "engine/SignalGraph.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

namespace noisefield::plugin
{

/// Plugin (VST3 / LV2 / CLAP) wrapper around `engine::SignalGraph`. Host automation goes
/// through an AudioProcessorValueTreeState which is mirrored into the graph's lock-free
/// parameter block once per block.
class NoisefieldAudioProcessor final : public juce::AudioProcessor
{
public:
    NoisefieldAudioProcessor();
    ~NoisefieldAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override
    {
        return true;
    }

    const juce::String getName() const override
    {
        return JucePlugin_Name;
    }

    bool acceptsMidi() const override
    {
        return false;
    }

    bool producesMidi() const override
    {
        return false;
    }

    bool isMidiEffect() const override
    {
        return false;
    }

    double getTailLengthSeconds() const override
    {
        return 0.0;
    }

    int getNumPrograms() override
    {
        return 1;
    }

    int getCurrentProgram() override
    {
        return 0;
    }

    void setCurrentProgram(int) override {}

    const juce::String getProgramName(int) override
    {
        return {};
    }

    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& apvts() noexcept
    {
        return apvts_;
    }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameterLayout();
    void syncParametersToGraph() noexcept;

    engine::SignalGraph graph_;
    juce::AudioProcessorValueTreeState apvts_;

    std::atomic<float>* masterGainDb_ = nullptr;
    std::atomic<float>* masterMute_ = nullptr;
    std::atomic<float>* limiter_ = nullptr;
    std::atomic<float>* toneEnabled_ = nullptr;
    std::atomic<float>* toneFrequencyHz_ = nullptr;
    std::atomic<float>* toneGainDb_ = nullptr;
    std::atomic<float>* noiseEnabled_ = nullptr;
    std::atomic<float>* noiseColour_ = nullptr;
    std::atomic<float>* noiseGainDb_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoisefieldAudioProcessor)
};

} // namespace noisefield::plugin
