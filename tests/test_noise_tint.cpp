#include "dsp/NoiseColour.h"
#include "dsp/NoiseTint.h"
#include "dsp/WhiteNoise.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

using noisefield::dsp::kNoiseColours;
using noisefield::dsp::NoiseColour;
using noisefield::dsp::NoiseTint;
using noisefield::dsp::WhiteNoise;

namespace
{
constexpr int kWarmup = 20000;
constexpr int kSamples = 400000;

double rmsOf(NoiseColour colour, std::uint64_t seed = 12345)
{
    WhiteNoise white(seed);
    NoiseTint tint;
    tint.setColour(colour);
    for (int i = 0; i < kWarmup; ++i)
        tint.process(white.nextSample());

    double sumSquares = 0.0;
    for (int i = 0; i < kSamples; ++i)
    {
        const double v = tint.process(white.nextSample());
        sumSquares += v * v;
    }
    return std::sqrt(sumSquares / kSamples);
}

// First-difference energy over signal energy: ~2 for white, lower for pink/brown (darker),
// higher for blue/violet (brighter). A crude, FFT-free spectral-tilt proxy.
double brightnessOf(NoiseColour colour, std::uint64_t seed = 999)
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
    return sumDiffSquares / sumSquares;
}
} // namespace

TEST_CASE("NoiseTint White is a pass-through", "[dsp]")
{
    WhiteNoise white(1);
    NoiseTint tint; // defaults to White
    for (int i = 0; i < 1000; ++i)
    {
        const float w = white.nextSample();
        REQUIRE(tint.process(w) == w);
    }
}

TEST_CASE("NoiseTint output is bounded and finite for every colour", "[dsp]")
{
    for (const auto colour : kNoiseColours)
    {
        WhiteNoise white(3);
        NoiseTint tint;
        tint.setColour(colour);
        for (int i = 0; i < 200000; ++i)
        {
            const float v = tint.process(white.nextSample());
            REQUIRE(std::isfinite(v));
            REQUIRE(std::abs(v) < 6.0f);
        }
    }
}

TEST_CASE("NoiseTint keeps every colour near the white input level", "[dsp]")
{
    // Calibration reference: gains in NoiseTint.h are tuned so rms/white is ~1.00 over a
    // 4 M-sample run. Here a shorter run, so a looser tolerance.
    const double whiteRms = rmsOf(NoiseColour::White);
    for (const auto colour : kNoiseColours)
    {
        const double ratio = rmsOf(colour) / whiteRms;
        INFO("colour = " << noiseColourName(colour) << "  ratio = " << ratio);
        REQUIRE(ratio > 0.8);
        REQUIRE(ratio < 1.2);
    }
}

TEST_CASE("NoiseTint spectral tilt is ordered dark -> bright", "[dsp]")
{
    const double brown = brightnessOf(NoiseColour::Brown);
    const double pink = brightnessOf(NoiseColour::Pink);
    const double white = brightnessOf(NoiseColour::White);
    const double blue = brightnessOf(NoiseColour::Blue);
    const double violet = brightnessOf(NoiseColour::Violet);
    const double grey = brightnessOf(NoiseColour::Grey);

    REQUIRE(brown < pink);
    REQUIRE(pink < white);
    REQUIRE(white < blue);
    REQUIRE(blue < violet);
    REQUIRE_THAT(white, Catch::Matchers::WithinAbs(2.0, 0.1)); // sanity: white really is flat
    REQUIRE(grey > pink);                                      // grey is not a dark noise
}

TEST_CASE("NoiseTint is deterministic per colour and white seed", "[dsp]")
{
    for (const auto colour : kNoiseColours)
    {
        WhiteNoise wa(555);
        WhiteNoise wb(555);
        NoiseTint a;
        NoiseTint b;
        a.setColour(colour);
        b.setColour(colour);
        for (int i = 0; i < 5000; ++i)
            REQUIRE(a.process(wa.nextSample()) == b.process(wb.nextSample()));
    }
}

TEST_CASE("NoiseTint setColour resets filter state", "[dsp]")
{
    WhiteNoise white(2);

    NoiseTint a;
    a.setColour(NoiseColour::Pink);
    for (int i = 0; i < 10000; ++i) // build up internal state
        a.process(white.nextSample());
    a.setColour(NoiseColour::Brown);
    a.setColour(NoiseColour::Pink); // back to pink, state should be cleared

    NoiseTint fresh;
    fresh.setColour(NoiseColour::Pink);

    WhiteNoise wa(77);
    WhiteNoise wf(77);
    for (int i = 0; i < 2000; ++i)
        REQUIRE(a.process(wa.nextSample()) == fresh.process(wf.nextSample()));
}
