#pragma once

#include "engine/SignalGraph.h"

#include <juce_audio_devices/juce_audio_devices.h>

#include <atomic>

namespace noisefield::engine
{

/// Standalone-app front end for `SignalGraph`: owns the audio device and drives the graph
/// from the real-time callback. The plugin build uses `SignalGraph` directly instead.
class AudioEngine final : private juce::AudioIODeviceCallback
{
public:
    AudioEngine();
    ~AudioEngine() override;

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    /// Opens the audio device (restoring `savedState` if given) and starts the callback.
    /// Returns an empty string on success, otherwise the device manager's error message.
    juce::String initialise(const juce::XmlElement* savedState);
    void shutdown();

    EngineParameters& parameters() noexcept
    {
        return graph_.parameters();
    }

    juce::AudioDeviceManager& deviceManager() noexcept
    {
        return deviceManager_;
    }

    MeterSnapshot fetchMeterAndReset() noexcept
    {
        return graph_.fetchMeterAndReset();
    }

    void readScope(float* dst, int count) noexcept
    {
        graph_.readScope(dst, count);
    }

    /// Underruns/overruns reported by the device since it opened, or -1 if unsupported.
    [[nodiscard]] int xRunCount() const noexcept;

    [[nodiscard]] double sampleRate() const noexcept
    {
        return graph_.sampleRate();
    }

    /// True after the device stopped on its own (e.g. the JACK server was killed) rather than
    /// via `shutdown()`. The caller (GUI thread) polls this and drives `attemptReconnect()`.
    [[nodiscard]] bool deviceLost() const noexcept
    {
        return deviceLost_.load(std::memory_order_relaxed);
    }

    /// Tries to reopen the last-used audio device (NF-073: JACK server restarts do not
    /// auto-reconnect otherwise). Safe to call repeatedly; does nothing if not `deviceLost()`.
    /// Called from the GUI thread, never from the audio callback.
    void attemptReconnect();

private:
    void
    audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                     int numInputChannels,
                                     float* const* outputChannelData,
                                     int numOutputChannels,
                                     int numSamples,
                                     const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

    juce::AudioDeviceManager deviceManager_;
    SignalGraph graph_;
    std::atomic<bool> shuttingDown_{false};
    std::atomic<bool> deviceLost_{false};
};

} // namespace noisefield::engine
