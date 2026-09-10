#include "engine/SignalGraph.h"

#include "dsp/Gain.h"
#include "dsp/LevelDetector.h"
#include "dsp/NoiseColour.h"

#include <algorithm>

namespace noisefield::engine
{

namespace
{
constexpr double kGainRampSeconds = 0.02;

dsp::NoiseColour colourFromIndex(int index) noexcept
{
    if (index >= 0 && index < static_cast<int>(dsp::kNoiseColours.size()))
        return dsp::kNoiseColours[static_cast<std::size_t>(index)];
    return dsp::NoiseColour::White;
}
} // namespace

void SignalGraph::prepare(double sampleRate, int maxBlockSize)
{
    sampleRate_.store(sampleRate, std::memory_order_relaxed);

    oscillator_.prepare(sampleRate);
    oscillator_.setFrequencyImmediate(params_.toneFrequencyHz.load(std::memory_order_relaxed));

    appliedNoiseSeed_ = params_.noiseSeed.load(std::memory_order_relaxed);
    noise_.setSeed(appliedNoiseSeed_);
    noiseTint_.setColour(colourFromIndex(params_.noiseColour.load(std::memory_order_relaxed)));
    noiseTint_.reset();
    scope_.reset();

    for (auto* smoother : {&toneGain_, &noiseGain_, &masterGain_})
    {
        smoother->prepare(sampleRate, kGainRampSeconds);
        smoother->setCurrentAndTarget(0.0f);
    }

    scratch_.assign(static_cast<std::size_t>(std::max(maxBlockSize, 0)), 0.0f);
}

void SignalGraph::reset() noexcept
{
    sampleRate_.store(0.0, std::memory_order_relaxed);
    meterPeak_.store(0.0f, std::memory_order_relaxed);
    meterRms_.store(0.0f, std::memory_order_relaxed);
    scope_.reset();
}

MeterSnapshot SignalGraph::fetchMeterAndReset() noexcept
{
    MeterSnapshot snapshot;
    snapshot.peak = meterPeak_.exchange(0.0f, std::memory_order_relaxed);
    snapshot.rms = meterRms_.load(std::memory_order_relaxed);
    return snapshot;
}

void SignalGraph::publishLevel(const float* block, int numSamples) noexcept
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

void SignalGraph::process(float* const* outputChannels,
                          int numOutputChannels,
                          int numSamples) noexcept
{
    // Denormal protection is the caller's job (they wrap this in juce::ScopedNoDenormals) so
    // that SignalGraph itself stays free of any framework dependency.
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
    noiseTint_.setColour(colourFromIndex(params_.noiseColour.load(std::memory_order_relaxed)));

    const int frames = std::min(numSamples, static_cast<int>(scratch_.size()));

    for (int i = 0; i < frames; ++i)
    {
        const float tone = oscillator_.nextSample() * toneGain_.nextValue();
        const float hiss = noiseTint_.process(noise_.nextSample()) * noiseGain_.nextValue();
        float sample = (tone + hiss) * masterGain_.nextValue();
        if (limiterOn)
            sample = limiter_.process(sample);
        scratch_[static_cast<std::size_t>(i)] = sample;
    }

    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (auto* out = outputChannels[channel])
        {
            std::copy(scratch_.begin(), scratch_.begin() + frames, out);
            if (frames < numSamples)
                std::fill(out + frames, out + numSamples, 0.0f);
        }
    }

    publishLevel(scratch_.data(), frames);
    scope_.write(scratch_.data(), frames);
}

} // namespace noisefield::engine
