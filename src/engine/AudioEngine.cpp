#include "engine/AudioEngine.h"

#include "dsp/Gain.h"
#include "dsp/LevelDetector.h"

#include <algorithm>

namespace noisefield::engine
{

namespace
{
constexpr double kGainRampSeconds = 0.02;
} // namespace

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
    running_.store(false, std::memory_order_relaxed);
}

int AudioEngine::xRunCount() const noexcept
{
    return deviceManager_.getXRunCount();
}

MeterSnapshot AudioEngine::fetchMeterAndReset() noexcept
{
    MeterSnapshot snapshot;
    snapshot.peak = meterPeak_.exchange(0.0f, std::memory_order_relaxed);
    snapshot.rms = meterRms_.load(std::memory_order_relaxed);
    return snapshot;
}

void AudioEngine::prepare(double sampleRate, int maxBlockSize)
{
    sampleRate_.store(sampleRate, std::memory_order_relaxed);

    oscillator_.prepare(sampleRate);
    oscillator_.setFrequencyImmediate(params_.toneFrequencyHz.load(std::memory_order_relaxed));

    appliedNoiseSeed_ = params_.noiseSeed.load(std::memory_order_relaxed);
    noise_.setSeed(appliedNoiseSeed_);

    for (auto* smoother : {&toneGain_, &noiseGain_, &masterGain_})
    {
        smoother->prepare(sampleRate, kGainRampSeconds);
        smoother->setCurrentAndTarget(0.0f);
    }

    scratch_.assign(static_cast<size_t>(std::max(maxBlockSize, 0)), 0.0f);
}

void AudioEngine::publishLevel(const float* block, int numSamples) noexcept
{
    const auto level = dsp::measureBlock(block, numSamples);

    float previousPeak = meterPeak_.load(std::memory_order_relaxed);
    while (level.peak > previousPeak &&
           !meterPeak_.compare_exchange_weak(previousPeak, level.peak, std::memory_order_relaxed))
    {
        // previousPeak reloaded by compare_exchange_weak; retry.
    }
    meterRms_.store(level.rms, std::memory_order_relaxed);
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

    const bool playing = params_.playing.load(std::memory_order_relaxed);
    const bool toneOn = playing && params_.toneEnabled.load(std::memory_order_relaxed);
    const bool noiseOn = playing && params_.noiseEnabled.load(std::memory_order_relaxed);
    const bool limiterOn = params_.limiterEnabled.load(std::memory_order_relaxed);
    const bool masterOn = playing && !params_.masterMute.load(std::memory_order_relaxed);

    toneGain_.setTarget(toneOn ? dsp::dbToGain(params_.toneGainDb.load(std::memory_order_relaxed))
                               : 0.0f);
    noiseGain_.setTarget(
        noiseOn ? dsp::dbToGain(params_.noiseGainDb.load(std::memory_order_relaxed)) : 0.0f);
    masterGain_.setTarget(
        masterOn ? dsp::dbToGain(params_.masterGainDb.load(std::memory_order_relaxed)) : 0.0f);

    oscillator_.setFrequency(params_.toneFrequencyHz.load(std::memory_order_relaxed));

    const auto requestedSeed = params_.noiseSeed.load(std::memory_order_relaxed);
    if (requestedSeed != appliedNoiseSeed_)
    {
        noise_.setSeed(requestedSeed);
        appliedNoiseSeed_ = requestedSeed;
    }

    const int frames = std::min(numSamples, static_cast<int>(scratch_.size()));

    for (int i = 0; i < frames; ++i)
    {
        const float tone = oscillator_.nextSample() * toneGain_.nextValue();
        const float hiss = noise_.nextSample() * noiseGain_.nextValue();
        float sample = (tone + hiss) * masterGain_.nextValue();
        if (limiterOn)
            sample = limiter_.process(sample);
        scratch_[static_cast<size_t>(i)] = sample;
    }

    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (auto* out = outputChannelData[channel])
        {
            std::copy(scratch_.begin(), scratch_.begin() + frames, out);
            if (frames < numSamples)
                juce::FloatVectorOperations::clear(out + frames, numSamples - frames);
        }
    }

    publishLevel(scratch_.data(), frames);
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    const double sampleRate = device != nullptr ? device->getCurrentSampleRate() : 44100.0;
    const int blockSize = device != nullptr ? device->getCurrentBufferSizeSamples() : 512;
    prepare(sampleRate, blockSize);
    running_.store(true, std::memory_order_relaxed);
}

void AudioEngine::audioDeviceStopped()
{
    running_.store(false, std::memory_order_relaxed);
    sampleRate_.store(0.0, std::memory_order_relaxed);
    meterPeak_.store(0.0f, std::memory_order_relaxed);
    meterRms_.store(0.0f, std::memory_order_relaxed);
}

} // namespace noisefield::engine
