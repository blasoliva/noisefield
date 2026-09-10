#pragma once

#include <array>
#include <string_view>

namespace noisefield::dsp
{

/// Noise "colours" Noisefield can generate. `NoiseTint` (NoiseTint.h) shapes a white stream
/// into any of these.
enum class NoiseColour
{
    White,  ///< flat power spectral density
    Pink,   ///< -3 dB/octave
    Brown,  ///< -6 dB/octave (leaky integrator)
    Blue,   ///< +3 dB/octave
    Violet, ///< +6 dB/octave
    Grey,   ///< perceptually flat (approximate inverse equal-loudness weighting)
};

inline constexpr std::array<NoiseColour, 6> kNoiseColours{
    NoiseColour::White,
    NoiseColour::Pink,
    NoiseColour::Brown,
    NoiseColour::Blue,
    NoiseColour::Violet,
    NoiseColour::Grey,
};

inline constexpr const char* noiseColourName(NoiseColour colour) noexcept
{
    switch (colour)
    {
    case NoiseColour::White:
        return "White";
    case NoiseColour::Pink:
        return "Pink";
    case NoiseColour::Brown:
        return "Brown";
    case NoiseColour::Blue:
        return "Blue";
    case NoiseColour::Violet:
        return "Violet";
    case NoiseColour::Grey:
        return "Grey";
    }
    return "White";
}

/// Inverse of noiseColourName(); falls back to White for anything unrecognised.
inline constexpr NoiseColour noiseColourFromName(std::string_view name) noexcept
{
    for (const auto colour : kNoiseColours)
        if (name == noiseColourName(colour))
            return colour;
    return NoiseColour::White;
}

} // namespace noisefield::dsp
