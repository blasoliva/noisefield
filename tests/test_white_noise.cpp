#include "dsp/WhiteNoise.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

using noisefield::dsp::WhiteNoise;

TEST_CASE("WhiteNoise output is bounded and roughly zero-mean", "[dsp]")
{
    WhiteNoise noise(42);

    double sum = 0.0;
    constexpr int n = 200000;
    for (int i = 0; i < n; ++i)
    {
        const float s = noise.nextSample();
        REQUIRE(s >= -1.0f);
        REQUIRE(s < 1.0f);
        sum += static_cast<double>(s);
    }

    const double mean = sum / n;
    REQUIRE_THAT(mean, Catch::Matchers::WithinAbs(0.0, 0.01));
}

TEST_CASE("WhiteNoise has a plausible variance for a uniform source", "[dsp]")
{
    WhiteNoise noise(7);

    double sumSquares = 0.0;
    constexpr int n = 200000;
    for (int i = 0; i < n; ++i)
    {
        const double s = noise.nextSample();
        sumSquares += s * s;
    }

    // Variance of a uniform distribution on [-1, 1] is 1/3.
    const double variance = sumSquares / n;
    REQUIRE_THAT(variance, Catch::Matchers::WithinAbs(1.0 / 3.0, 0.02));
}

TEST_CASE("WhiteNoise is deterministic per seed", "[dsp]")
{
    WhiteNoise a(123);
    WhiteNoise b(123);
    WhiteNoise c(124);

    bool differsFromC = false;
    for (int i = 0; i < 1000; ++i)
    {
        const float sa = a.nextSample();
        REQUIRE(sa == b.nextSample());
        if (sa != c.nextSample())
            differsFromC = true;
    }

    REQUIRE(differsFromC);
}

TEST_CASE("WhiteNoise is spectrally flat with negligible sample-to-sample correlation", "[dsp]")
{
    WhiteNoise noise(99);

    constexpr int n = 400000;
    double sumSquares = 0.0;
    double sumDiffSquares = 0.0;
    float previous = noise.nextSample();
    for (int i = 1; i < n; ++i)
    {
        const float current = noise.nextSample();
        const double value = static_cast<double>(current);
        sumSquares += value * value;
        const double diff = value - static_cast<double>(previous);
        sumDiffSquares += diff * diff;
        previous = current;
    }

    // For a flat spectrum E[(x[n]-x[n-1])^2] == 2 * variance (autocorrelation at lag 1 is 0).
    // Pink/brown noise would give a ratio well below 2; blue/violet well above.
    const double ratio = sumDiffSquares / sumSquares;
    REQUIRE_THAT(ratio, Catch::Matchers::WithinAbs(2.0, 0.05));
}
