#pragma once

#include "dsp/NoiseColour.h"

#include <cstdint>
#include <string>

namespace noisefield::model
{

/// The full user-adjustable sound state: what a preset saves and recalls, and what a `.nfp`
/// file holds. Flat while the engine has one tone + one noise source; a `layers` array
/// arrives with the M3 engine rework (schema v2 — bump `kSchemaVersion` and migrate).
struct Preset
{
    static constexpr int kSchemaVersion = 1;

    std::string name = "Untitled";

    bool toneEnabled = true;
    double toneFrequencyHz = 220.0;
    double toneGainDb = -14.0;

    bool noiseEnabled = false;
    dsp::NoiseColour noiseColour = dsp::NoiseColour::White;
    double noiseGainDb = -20.0;
    std::uint64_t noiseSeed = 1;

    bool masterMute = false;
    double masterGainDb = -6.0;
    bool limiterEnabled = true;
};

} // namespace noisefield::model
