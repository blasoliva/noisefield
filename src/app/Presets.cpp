#include "app/Presets.h"

#include "dsp/NoiseColour.h"

#include <string>
#include <utility>

namespace noisefield::app
{

namespace
{
using noisefield::dsp::NoiseColour;

model::Preset make(std::string name,
                   bool toneOn,
                   double freq,
                   double toneDb,
                   bool noiseOn,
                   NoiseColour colour,
                   double noiseDb,
                   double masterDb)
{
    model::Preset p;
    p.name = std::move(name);
    p.toneEnabled = toneOn;
    p.toneFrequencyHz = freq;
    p.toneGainDb = toneDb;
    p.noiseEnabled = noiseOn;
    p.noiseColour = colour;
    p.noiseGainDb = noiseDb;
    p.masterGainDb = masterDb;
    p.limiterEnabled = true;
    return p;
}
} // namespace

const std::vector<model::Preset>& factoryPresets()
{
    static const std::vector<model::Preset> presets = {
        make("Tinnitus mask", false, 4000.0, -28.0, true, NoiseColour::Pink, -12.0, -6.0),
        make("Tinnitus mask + tone", true, 4000.0, -26.0, true, NoiseColour::Pink, -14.0, -6.0),
        make("Focus", false, 1000.0, -24.0, true, NoiseColour::Pink, -14.0, -6.0),
        make("Sleep", true, 60.0, -30.0, true, NoiseColour::Brown, -12.0, -8.0),
        make("Deep rain", false, 1000.0, -24.0, true, NoiseColour::Brown, -10.0, -6.0),
        make("White wash", false, 1000.0, -24.0, true, NoiseColour::White, -12.0, -6.0),
        make("Test tone 1 kHz", true, 1000.0, -12.0, false, NoiseColour::White, -20.0, -6.0),
        make("Silence", false, 1000.0, -60.0, false, NoiseColour::White, -60.0, -6.0),
    };
    return presets;
}

} // namespace noisefield::app
