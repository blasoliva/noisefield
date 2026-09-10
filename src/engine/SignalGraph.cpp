#include "engine/SignalGraph.h"

#include "dsp/Gain.h"
#include "dsp/LevelDetector.h"
#include "dsp/NoiseColour.h"

#include <algorithm>
#include <cstddef>

namespace noisefield::engine
{

namespace
{
constexpr double kMasterRampSeconds = 0.02;
constexpr double kLayerRampSeconds = 0.04; // add/remove crossfades stay inaudible

dsp::NoiseColour colourFromIndex(int index) noexcept
{
    if (index >= 0 && index < static_cast<int>(dsp::kNoiseColours.size()))
        return dsp::kNoiseColours[static_cast<std::size_t>(index)];
    return dsp::NoiseColour::White;
}
} // namespace

void SignalGraph::prepareVoice(LayerVoice& voice, const LayerParameters& params) noexcept
{
    const double sampleRate = sampleRate_.load(std::memory_order_relaxed);

    voice.oscillator.prepare(sampleRate);
    voice.oscillator.setFrequencyImmediate(params.frequencyHz.load(std::memory_order_relaxed));
    voice.oscillator.reset();

    voice.appliedSeed = params.seed.load(std::memory_order_relaxed);
    voice.noise.setSeed(voice.appliedSeed);
    voice.tint.setColour(colourFromIndex(params.noiseColour.load(std::memory_order_relaxed)));
    voice.tint.reset();

    voice.appliedEpoch = params.epoch.load(std::memory_order_relaxed);

    voice.gain.prepare(sampleRate, kLayerRampSeconds);
    voice.gain.setCurrentAndTarget(0.0f);
}

void SignalGraph::prepare(double sampleRate, int maxBlockSize)
{
    sampleRate_.store(sampleRate, std::memory_order_relaxed);

    for (std::size_t i = 0; i < voices_.size(); ++i)
        prepareVoice(voices_[i], params_.layers[i]);

    masterGain_.prepare(sampleRate, kMasterRampSeconds);
    masterGain_.setCurrentAndTarget(0.0f);

    scope_.reset();
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

void SignalGraph::renderLayer(LayerVoice& voice, const LayerParameters& params, int frames) noexcept
{
    const bool playing = params_.playing.load(std::memory_order_relaxed);
    const bool active = params.active.load(std::memory_order_relaxed);
    const bool muted = params.muted.load(std::memory_order_relaxed);
    const auto epoch = params.epoch.load(std::memory_order_relaxed);
    const auto source = static_cast<LayerSource>(params.source.load(std::memory_order_relaxed));
    const auto seed = params.seed.load(std::memory_order_relaxed);
    const auto colour = colourFromIndex(params.noiseColour.load(std::memory_order_relaxed));
    const float frequencyHz = params.frequencyHz.load(std::memory_order_relaxed);
    const float gainDb = params.gainDb.load(std::memory_order_relaxed);

    if (epoch != voice.appliedEpoch)
    {
        // Slot (re)assigned: jump the voice to the new configuration with the gain at zero so
        // it fades in cleanly and no tail of the previous layer leaks through.
        voice.oscillator.setFrequencyImmediate(frequencyHz);
        voice.oscillator.reset();
        voice.noise.setSeed(seed);
        voice.tint.setColour(colour);
        voice.tint.reset();
        voice.gain.setCurrentAndTarget(0.0f);
        voice.appliedEpoch = epoch;
        voice.appliedSeed = seed;
    }
    else
    {
        voice.oscillator.setFrequency(frequencyHz); // ramped internally
        voice.tint.setColour(colour);               // no-op if unchanged
        if (seed != voice.appliedSeed)
        {
            voice.noise.setSeed(seed);
            voice.appliedSeed = seed;
        }
    }

    voice.gain.setTarget((active && !muted && playing) ? dsp::dbToGain(gainDb) : 0.0f);

    // Idle slot: contributes nothing and is not ramping -- skip it entirely (no CPU, and the
    // oscillator/noise state stays frozen until the slot is used again).
    if (!active && voice.gain.current() == 0.0f && !voice.gain.isSmoothing())
        return;

    for (int i = 0; i < frames; ++i)
    {
        const float raw = source == LayerSource::Oscillator
                              ? voice.oscillator.nextSample()
                              : voice.tint.process(voice.noise.nextSample());
        scratch_[static_cast<std::size_t>(i)] += raw * voice.gain.nextValue();
    }
}

void SignalGraph::process(float* const* outputChannels,
                          int numOutputChannels,
                          int numSamples) noexcept
{
    // Denormal protection is the caller's job (they wrap this in juce::ScopedNoDenormals) so
    // that SignalGraph itself stays free of any framework dependency.
    const bool playing = params_.playing.load(std::memory_order_relaxed);
    const bool limiterOn = params_.limiterEnabled.load(std::memory_order_relaxed);
    const bool masterOn = playing && !params_.masterMute.load(std::memory_order_relaxed);

    masterGain_.setTarget(
        masterOn ? dsp::dbToGain(params_.masterGainDb.load(std::memory_order_relaxed)) : 0.0f);

    const int frames = std::min(numSamples, static_cast<int>(scratch_.size()));

    std::fill(scratch_.begin(), scratch_.begin() + frames, 0.0f);
    for (std::size_t i = 0; i < voices_.size(); ++i)
        renderLayer(voices_[i], params_.layers[i], frames);

    for (int i = 0; i < frames; ++i)
    {
        float sample = scratch_[static_cast<std::size_t>(i)] * masterGain_.nextValue();
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
