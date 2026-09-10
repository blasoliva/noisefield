#pragma once

#include "dsp/Constants.h"
#include "dsp/ParamSmoother.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace noisefield::dsp
{

/// Sine oscillator with a click-free frequency control. `setFrequency()` ramps toward the new
/// value; `setFrequencyImmediate()` jumps. Frequency is clamped to [kMinFrequencyHz,
/// kMaxFrequencyHz]. Phase stays continuous across frequency changes.
class SineOscillator
{
public:
    SineOscillator() noexcept
    {
        frequency_.setCurrentAndTarget(static_cast<float>(kDefaultFrequencyHz));
    }

    void prepare(double sampleRate) noexcept
    {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
        frequency_.prepare(sampleRate_, kRampSeconds);
        updateIncrement(frequency_.current());
    }

    void setFrequency(double hz) noexcept
    {
        frequency_.setTarget(static_cast<float>(clampFrequency(hz)));
    }

    void setFrequencyImmediate(double hz) noexcept
    {
        frequency_.setCurrentAndTarget(static_cast<float>(clampFrequency(hz)));
        updateIncrement(frequency_.current());
    }

    void reset() noexcept { phase_ = 0.0; }

    /// Advances by one sample and returns the output in [-1, 1].
    float nextSample() noexcept
    {
        if (frequency_.isSmoothing())
            updateIncrement(frequency_.nextValue());

        const auto value = std::sin(phase_);
        phase_ += increment_;
        if (phase_ >= kTwoPi)
            phase_ -= kTwoPi;
        return static_cast<float>(value);
    }

    [[nodiscard]] double frequencyHz() const noexcept
    {
        return static_cast<double>(frequency_.current());
    }

private:
    static constexpr double kTwoPi = 2.0 * std::numbers::pi;
    static constexpr double kRampSeconds = 0.03;
    static constexpr double kDefaultFrequencyHz = 440.0;

    static double clampFrequency(double hz) noexcept
    {
        return std::clamp(hz, kMinFrequencyHz, kMaxFrequencyHz);
    }

    void updateIncrement(double frequencyHz) noexcept
    {
        increment_ = kTwoPi * frequencyHz / sampleRate_;
    }

    double sampleRate_ = 44100.0;
    double phase_ = 0.0;
    double increment_ = kTwoPi * kDefaultFrequencyHz / 44100.0;
    ParamSmoother frequency_;
};

} // namespace noisefield::dsp
