#include "dsp/Constants.h"
#include "dsp/SineOscillator.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

using noisefield::dsp::SineOscillator;

namespace
{
int countUpwardZeroCrossings(SineOscillator& osc, int numSamples)
{
    int crossings = 0;
    float previous = osc.nextSample();
    for (int i = 1; i < numSamples; ++i)
    {
        const float current = osc.nextSample();
        if (previous < 0.0f && current >= 0.0f)
            ++crossings;
        previous = current;
    }
    return crossings;
}
} // namespace

TEST_CASE("SineOscillator output stays in the unit range", "[dsp]")
{
    SineOscillator osc;
    osc.prepare(48000.0);
    osc.setFrequencyImmediate(997.0);

    for (int i = 0; i < 48000; ++i)
    {
        const float s = osc.nextSample();
        REQUIRE(s >= -1.0f);
        REQUIRE(s <= 1.0f);
    }
}

TEST_CASE("SineOscillator has the expected RMS for a full-scale sine", "[dsp]")
{
    SineOscillator osc;
    osc.prepare(48000.0);
    osc.setFrequencyImmediate(1000.0);

    double sumSquares = 0.0;
    constexpr int n = 48000;
    for (int i = 0; i < n; ++i)
    {
        const double s = osc.nextSample();
        sumSquares += s * s;
    }

    const double rms = std::sqrt(sumSquares / n);
    REQUIRE_THAT(rms, Catch::Matchers::WithinAbs(0.70710678, 0.01));
}

TEST_CASE("SineOscillator produces the requested frequency", "[dsp]")
{
    SineOscillator osc;
    osc.prepare(48000.0);
    osc.setFrequencyImmediate(1000.0);

    const int crossings = countUpwardZeroCrossings(osc, 48000);
    REQUIRE(crossings >= 999);
    REQUIRE(crossings <= 1001);
}

TEST_CASE("SineOscillator clamps frequency to the audible range", "[dsp]")
{
    SineOscillator osc;
    osc.prepare(48000.0);

    osc.setFrequencyImmediate(5.0);
    REQUIRE(osc.frequencyHz() == noisefield::dsp::kMinFrequencyHz);

    osc.setFrequencyImmediate(50000.0);
    REQUIRE(osc.frequencyHz() == noisefield::dsp::kMaxFrequencyHz);
}

TEST_CASE("SineOscillator ramps frequency changes without a discontinuity", "[dsp]")
{
    SineOscillator osc;
    osc.prepare(48000.0);
    osc.setFrequencyImmediate(100.0);

    // Prime, then request a large jump.
    for (int i = 0; i < 480; ++i)
        osc.nextSample();
    osc.setFrequency(8000.0);

    float previous = osc.nextSample();
    float maxDelta = 0.0f;
    for (int i = 0; i < 4800; ++i) // 100 ms, longer than the 30 ms ramp
    {
        const float current = osc.nextSample();
        maxDelta = std::max(maxDelta, std::abs(current - previous));
        previous = current;
    }

    // A clean 8 kHz sine at 48 kHz moves at most ~0.9 per sample; a hard frequency jump would
    // still be continuous in amplitude, so this mainly guards against blow-ups / NaNs.
    REQUIRE(std::isfinite(maxDelta));
    REQUIRE(maxDelta < 1.5f);

    // After the ramp it should sit at the requested frequency.
    const int crossings = countUpwardZeroCrossings(osc, 48000);
    REQUIRE(crossings >= 7960);
    REQUIRE(crossings <= 8040);
}
