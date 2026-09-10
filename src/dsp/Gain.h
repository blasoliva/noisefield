#pragma once

#include "dsp/Constants.h"

#include <cmath>

namespace noisefield::dsp
{

/// Converts decibels to a linear amplitude factor. At or below `kMinGainDb` the result is
/// exactly 0 so a fader taken to its bottom is truly silent.
inline float dbToGain(float decibels, float minusInfinityDb = kMinGainDb) noexcept
{
    return decibels <= minusInfinityDb ? 0.0f : std::pow(10.0f, decibels * 0.05f);
}

inline float gainToDb(float gain, float minusInfinityDb = kMinGainDb) noexcept
{
    return gain > 0.0f ? 20.0f * std::log10(gain) : minusInfinityDb;
}

} // namespace noisefield::dsp
