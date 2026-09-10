#pragma once

#include "dsp/NoiseTint.h"
#include "dsp/ParamSmoother.h"
#include "dsp/SineOscillator.h"
#include "dsp/SoftLimiter.h"
#include "dsp/WhiteNoise.h"
#include "engine/EngineParameters.h"

#include <juce_audio_devices/juce_audio_devices.h>

#include <atomic>
#include <vector>

namespace noisefield::engine
{

/// Owns the audio device and the real-time signal path: one sine oscillator plus one white
/// noise source, mixed with smoothed gains into a master bus with an optional soft limiter.
///
/// Real-time rule for everything reachable from the callback: no allocation, no locks, no
/// I/O, no exceptions. See docs/realtime-rules.md.
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
        return params_;
    }

    juce::AudioDeviceManager& deviceManager() noexcept
    {
        return deviceManager_;
    }

    /// Reads the master level and resets the peak hold. Lock-free; call from the GUI thread.
    MeterSnapshot fetchMeterAndReset() noexcept;

    /// Underruns/overruns reported by the device since it opened, or -1 if unsupported.
    [[nodiscard]] int xRunCount() const noexcept;

    [[nodiscard]] double sampleRate() const noexcept
    {
        return sampleRate_.load(std::memory_order_relaxed);
    }

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

    void prepare(double sampleRate, int maxBlockSize);
    void publishLevel(const float* block, int numSamples) noexcept;

    juce::AudioDeviceManager deviceManager_;
    EngineParameters params_;

    dsp::SineOscillator oscillator_;
    dsp::WhiteNoise noise_;
    dsp::NoiseTint noiseTint_;
    dsp::SoftLimiter limiter_;
    dsp::ParamSmoother toneGain_;
    dsp::ParamSmoother noiseGain_;
    dsp::ParamSmoother masterGain_;

    std::vector<float> scratch_;
    std::uint64_t appliedNoiseSeed_ = 1;

    std::atomic<double> sampleRate_{0.0};
    std::atomic<float> meterPeak_{0.0f};
    std::atomic<float> meterRms_{0.0f};
    std::atomic<bool> running_{false};
};

} // namespace noisefield::engine
