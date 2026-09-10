#pragma once

#include <cmath>

namespace noisefield::dsp
{

/// Soft limiter for the master bus. Signal below `threshold` passes through untouched; above
/// it, the excess is compressed through tanh so the output stays strictly inside (-1, 1)
/// without the harsh artefacts of hard digital clipping.
///
/// The transfer function is C1-continuous at the threshold (both sides have slope 1 there).
class SoftLimiter
{
public:
    void setThreshold(float threshold) noexcept
    {
        threshold_ = threshold > 0.0f && threshold < 1.0f ? threshold : 0.891f; // ~ -1 dBFS
    }

    [[nodiscard]] float threshold() const noexcept { return threshold_; }

    [[nodiscard]] float process(float x) const noexcept
    {
        const float magnitude = std::abs(x);
        if (magnitude <= threshold_)
            return x;

        const float sign = x < 0.0f ? -1.0f : 1.0f;
        const float headroom = 1.0f - threshold_;
        const float over = magnitude - threshold_;
        return sign * (threshold_ + headroom * std::tanh(over / headroom));
    }

private:
    float threshold_ = 0.891f;
};

} // namespace noisefield::dsp
