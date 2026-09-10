#pragma once

#include <cmath>
#include <cstddef>

namespace noisefield::dsp
{

struct BlockLevel
{
    float peak = 0.0f; ///< max |sample| over the block
    float rms = 0.0f;  ///< root-mean-square over the block
};

/// Measures peak and RMS of a mono block. Allocation-free; safe on the audio thread.
inline BlockLevel measureBlock(const float* data, int numSamples) noexcept
{
    if (data == nullptr || numSamples <= 0)
        return {};

    float peak = 0.0f;
    double sumSquares = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        const float magnitude = std::abs(data[i]);
        if (magnitude > peak)
            peak = magnitude;
        sumSquares += static_cast<double>(data[i]) * static_cast<double>(data[i]);
    }

    return { peak, static_cast<float>(std::sqrt(sumSquares / numSamples)) };
}

} // namespace noisefield::dsp
