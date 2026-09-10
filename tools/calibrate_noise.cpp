// Measures each noise colour against the white input and reports the RMS ratio, a crude
// spectral-brightness proxy, and the gain-constant tweak that would re-match the level.
//
// Run it after changing any filter in src/dsp/NoiseTint.h, then update the kXxxGain
// constants so every "rms/white" column reads ~1.00.
//
//   c++ -O2 -std=c++20 -I src tools/calibrate_noise.cpp -o /tmp/calibrate_noise
//   /tmp/calibrate_noise

#include "dsp/NoiseColour.h"
#include "dsp/NoiseTint.h"
#include "dsp/WhiteNoise.h"

#include <cmath>
#include <cstdint>
#include <cstdio>

using namespace noisefield::dsp;

namespace
{
constexpr int kWarmup = 20000;
constexpr int kSamples = 4'000'000;

struct Stats
{
    double rms = 0.0;
    double brightness = 0.0; // sum((x[n]-x[n-1])^2) / sum(x^2): ~2 white, <2 dark, >2 bright
};

Stats measure(NoiseColour colour, std::uint64_t seed)
{
    WhiteNoise white(seed);
    NoiseTint tint;
    tint.setColour(colour);

    for (int i = 0; i < kWarmup; ++i)
        tint.process(white.nextSample());

    double sumSquares = 0.0;
    double sumDiffSquares = 0.0;
    double previous = tint.process(white.nextSample());
    for (int i = 1; i < kSamples; ++i)
    {
        const double v = tint.process(white.nextSample());
        sumSquares += v * v;
        const double d = v - previous;
        sumDiffSquares += d * d;
        previous = v;
    }

    Stats s;
    s.rms = std::sqrt(sumSquares / kSamples);
    s.brightness = sumDiffSquares / sumSquares;
    return s;
}
} // namespace

int main()
{
    const double whiteRms = measure(NoiseColour::White, 12345).rms;

    std::printf(
        "%-7s  %-10s  %-10s  %s\n", "colour", "rms/white", "brightness", "suggested gain x");
    std::printf("-------  ----------  ----------  ----------------\n");

    for (const auto colour : kNoiseColours)
    {
        const Stats s = measure(colour, 12345);
        const double ratio = s.rms / whiteRms;
        std::printf("%-7s  %-10.4f  %-10.3f  %s%.4f\n",
                    noiseColourName(colour),
                    ratio,
                    s.brightness,
                    colour == NoiseColour::White ? "  " : "x ",
                    colour == NoiseColour::White ? 1.0 : 1.0 / ratio);
    }

    std::printf("\nMultiply the matching kXxxGain in src/dsp/NoiseTint.h by the last column,\n"
                "then re-run until every rms/white reads ~1.00.\n");
    return 0;
}
