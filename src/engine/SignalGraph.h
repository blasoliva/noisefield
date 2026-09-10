#pragma once

#include "dsp/NoiseTint.h"
#include "dsp/ParamSmoother.h"
#include "dsp/ScopeBuffer.h"
#include "dsp/SineOscillator.h"
#include "dsp/SoftLimiter.h"
#include "dsp/WhiteNoise.h"
#include "engine/EngineParameters.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <vector>

namespace noisefield::engine
{

/// The real-time signal path shared by the standalone app and the plugin: a fixed pool of
/// layer voices (each a sine oscillator or a tinted white-noise source) mixed with smoothed
/// per-layer gains into a master bus with an optional soft limiter, tapped for the meter and
/// the oscilloscope.
///
/// Owns the parameter block. Callers write to `parameters()` from any thread (lock-free
/// atomics) and call `process()` from their audio thread. Everything reachable from
/// `process()` is allocation-free and lock-free; see docs/realtime-rules.md.
///
/// Layers are added/removed by toggling `LayerParameters::active`; the per-layer gain ramp
/// crossfades them in and out. Re-using a slot for a different layer is an `epoch` bump,
/// which hard-resets that voice. The mix is a plain sum, so slot order is irrelevant --
/// display order is the GUI's concern.
class SignalGraph
{
public:
    /// Call from the non-audio thread before streaming starts.
    void prepare(double sampleRate, int maxBlockSize);
    void reset() noexcept;

    /// Renders `numSamples` frames of the mono signal into every output channel.
    void process(float* const* outputChannels, int numOutputChannels, int numSamples) noexcept;

    EngineParameters& parameters() noexcept
    {
        return params_;
    }

    /// Reads the master level and resets the peak hold. Lock-free; call from the GUI thread.
    MeterSnapshot fetchMeterAndReset() noexcept;

    /// Copies the most recent `count` master-bus samples for the oscilloscope. Lock-free.
    void readScope(float* dst, int count) noexcept
    {
        scope_.readLatest(dst, count);
    }

    [[nodiscard]] double sampleRate() const noexcept
    {
        return sampleRate_.load(std::memory_order_relaxed);
    }

private:
    /// The real-time state for one layer slot. Header-only DSP primitives, no allocation.
    struct LayerVoice
    {
        dsp::SineOscillator oscillator;
        dsp::WhiteNoise noise;
        dsp::NoiseTint tint;
        dsp::ParamSmoother gain;
        std::uint64_t appliedEpoch = 0;
        std::uint64_t appliedSeed = 1;
    };

    void prepareVoice(LayerVoice& voice, const LayerParameters& params) noexcept;
    void renderLayer(LayerVoice& voice, const LayerParameters& params, int frames) noexcept;
    void publishLevel(const float* block, int numSamples) noexcept;

    EngineParameters params_;

    std::array<LayerVoice, kMaxLayers> voices_;
    dsp::SoftLimiter limiter_;
    dsp::ParamSmoother masterGain_;
    dsp::ScopeBuffer scope_;

    std::vector<float> scratch_;

    std::atomic<double> sampleRate_{0.0};
    std::atomic<float> meterPeak_{0.0f};
    std::atomic<float> meterRms_{0.0f};
};

} // namespace noisefield::engine
