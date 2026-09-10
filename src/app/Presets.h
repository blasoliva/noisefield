#pragma once

#include <array>

namespace noisefield::app
{

/// A named starting point applied to all controls at once. Not persisted itself — only the
/// resulting control values are. Colour indices match `dsp::kNoiseColours`
/// (0 white, 1 pink, 2 brown, 3 blue, 4 violet, 5 grey).
struct Preset
{
    const char* name;
    bool toneEnabled;
    double toneFrequencyHz;
    double toneGainDb;
    bool noiseEnabled;
    int noiseColour;
    double noiseGainDb;
    double masterGainDb;
    bool limiterEnabled;
};

inline constexpr std::array<Preset, 8> kPresets{{
    // name                 tone   freq     toneDb  noise  col  noiseDb  masterDb  lim
    {"Tinnitus mask", false, 4000.0, -28.0, true, 1, -12.0, -6.0, true},
    {"Tinnitus mask + tone", true, 4000.0, -26.0, true, 1, -14.0, -6.0, true},
    {"Focus", false, 1000.0, -24.0, true, 1, -14.0, -6.0, true},
    {"Sleep", true, 60.0, -30.0, true, 2, -12.0, -8.0, true},
    {"Deep rain", false, 1000.0, -24.0, true, 2, -10.0, -6.0, true},
    {"White wash", false, 1000.0, -24.0, true, 0, -12.0, -6.0, true},
    {"Test tone 1 kHz", true, 1000.0, -12.0, false, 0, -20.0, -6.0, true},
    {"Silence", false, 1000.0, -60.0, false, 0, -60.0, -6.0, true},
}};

} // namespace noisefield::app
