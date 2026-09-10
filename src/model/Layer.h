#pragma once

#include "dsp/NoiseColour.h"

#include <cstdint>
#include <string>

namespace noisefield::model
{

enum class LayerType
{
    Oscillator,
    Noise,
};

enum class OscillatorShape
{
    Sine,
    Triangle,
    Square,
    Saw,
};

enum class FilterMode
{
    Off,
    LowPass,
    HighPass,
    BandPass,
    Notch,
};

/// Per-layer band filter settings (SVF/TPT). Implemented in milestone M3.
struct BandFilter
{
    FilterMode mode = FilterMode::Off;
    double cutoffHz = 1000.0;
    double resonanceQ = 0.707;
};

/// One sound source in a project. This is a plain data struct; the audio engine builds its
/// real-time voices from it. Serialization to JSON lands in milestone M3 (NF-040).
struct Layer
{
    std::string name = "Layer";
    LayerType type = LayerType::Oscillator;

    // Oscillator parameters
    OscillatorShape shape = OscillatorShape::Sine;
    double frequencyHz = 440.0;
    double fineTuneCents = 0.0;

    // Noise parameters
    dsp::NoiseColour noiseColour = dsp::NoiseColour::White;
    std::uint64_t noiseSeed = 1;

    BandFilter filter {};

    // Mix parameters
    float gainDb = -12.0f;
    float pan = 0.0f; // -1 = left, +1 = right
    bool mute = false;
    bool solo = false;
};

} // namespace noisefield::model
