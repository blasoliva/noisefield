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
    shuttingDown_.store(true, std::memory_order_relaxed);
    deviceManager_.removeAudioCallback(this);
    deviceManager_.closeAudioDevice();
}

int AudioEngine::xRunCount() const noexcept
{
    return deviceManager_.getXRunCount();
}

void AudioEngine::attemptReconnect()
{
    if (!deviceLost_.load(std::memory_order_relaxed))
        return;
    deviceManager_.restartLastAudioDevice();
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
    deviceLost_.store(false, std::memory_order_relaxed);
    const double sampleRate = device != nullptr ? device->getCurrentSampleRate() : 44100.0;
    const int blockSize = device != nullptr ? device->getCurrentBufferSizeSamples() : 512;
    graph_.prepare(sampleRate, blockSize);
}

void AudioEngine::audioDeviceStopped()
{
    graph_.reset();
    // Distinguish an intentional close (shutdown()) from the device disappearing under us
    // (e.g. the JACK server was killed) so the GUI knows whether to try reconnecting.
    if (!shuttingDown_.load(std::memory_order_relaxed))
        deviceLost_.store(true, std::memory_order_relaxed);
}

void AudioEngine::audioDeviceError(const juce::String& /*errorMessage*/)
{
    if (!shuttingDown_.load(std::memory_order_relaxed))
        deviceLost_.store(true, std::memory_order_relaxed);
}

} // namespace noisefield::engine
