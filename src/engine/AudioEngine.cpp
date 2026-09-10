#include "engine/AudioEngine.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace noisefield::engine
{

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine()
{
    shutdown();
}

juce::String AudioEngine::initialise(const juce::XmlElement* savedState)
{
    const auto error = deviceManager_.initialise(0, 2, savedState, true);
    if (error.isNotEmpty())
        return error;

    deviceManager_.addAudioCallback(this);
    return {};
}

void AudioEngine::shutdown()
{
    deviceManager_.removeAudioCallback(this);
    deviceManager_.closeAudioDevice();
}

int AudioEngine::xRunCount() const noexcept
{
    return deviceManager_.getXRunCount();
}

void AudioEngine::audioDeviceIOCallbackWithContext(
    const float* const* /*inputChannelData*/,
    int /*numInputChannels*/,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& /*context*/)
{
    const juce::ScopedNoDenormals noDenormals;
    graph_.process(outputChannelData, numOutputChannels, numSamples);
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    const double sampleRate = device != nullptr ? device->getCurrentSampleRate() : 44100.0;
    const int blockSize = device != nullptr ? device->getCurrentBufferSizeSamples() : 512;
    graph_.prepare(sampleRate, blockSize);
}

void AudioEngine::audioDeviceStopped()
{
    graph_.reset();
}

} // namespace noisefield::engine
