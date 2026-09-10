#pragma once

#include "dsp/NoiseTint.h"
#include "dsp/ParamSmoother.h"
#include "dsp/ScopeBuffer.h"
#include "dsp/SineOscillator.h"
#include "dsp/SoftLimiter.h"
#include "dsp/WhiteNoise.h"
#include "engine/EngineParameters.h"

#include <atomic>
#include <cstdint>
#include <vector>

namespace noisefield::engine
{

/// The real-time signal path shared by the standalone app and the plugin: one sine oscillator
/// plus one tinted white-noise source, mixed with smoothed gains into a master bus with an
/// optional soft limiter, tapped for the meter and the oscilloscope.
///
/// Owns the parameter block. Callers write to `parameters()` from any thread (lock-free
/// atomics) and call `process()` from their audio thread. Everything reachable from
/// `process()` is allocation-free and lock-free; see docs/realtime-rules.md.
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
    void publishLevel(const float* block, int numSamples) noexcept;

    EngineParameters params_;

    dsp::SineOscillator oscillator_;
    dsp::WhiteNoise noise_;
    dsp::NoiseTint noiseTint_;
    dsp::SoftLimiter limiter_;
    dsp::ParamSmoother toneGain_;
    dsp::ParamSmoother noiseGain_;
    dsp::ParamSmoother masterGain_;
    dsp::ScopeBuffer scope_;

    std::vector<float> scratch_;
    std::uint64_t appliedNoiseSeed_ = 1;

    std::atomic<double> sampleRate_{0.0};
    std::atomic<float> meterPeak_{0.0f};
    std::atomic<float> meterRms_{0.0f};
};

} // namespace noisefield::engine
