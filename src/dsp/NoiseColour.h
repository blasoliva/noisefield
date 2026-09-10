#pragma once

namespace noisefield::dsp
{

/// Noise "colours" planned for milestone M3. Only White is implemented so far
/// (see WhiteNoise.h); the rest are placeholders to fix the vocabulary early.
enum class NoiseColour
{
    White,  ///< flat power spectral density
    Pink,   ///< -3 dB/octave
    Brown,  ///< -6 dB/octave (leaky integrator)
    Blue,   ///< +3 dB/octave
    Violet, ///< +6 dB/octave
    Grey,   ///< perceptually flat (inverse equal-loudness weighting)
};

} // namespace noisefield::dsp
