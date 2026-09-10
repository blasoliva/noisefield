#include "dsp/SoftLimiter.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

using noisefield::dsp::SoftLimiter;

TEST_CASE("SoftLimiter passes signal below the threshold unchanged", "[dsp]")
{
    SoftLimiter limiter;
    limiter.setThreshold(0.8f);

    for (float x = -0.79f; x <= 0.79f; x += 0.01f)
        REQUIRE_THAT(limiter.process(x), Catch::Matchers::WithinAbs(x, 1.0e-6));
}

TEST_CASE("SoftLimiter keeps the output within the unit interval", "[dsp]")
{
    SoftLimiter limiter;
    limiter.setThreshold(0.9f);

    for (float x : {-1000.0f, -3.0f, -1.0f, 1.0f, 3.0f, 1000.0f})
    {
        const float y = limiter.process(x);
        REQUIRE(std::abs(y) <= 1.0f);
        REQUIRE(std::isfinite(y));
    }

    // Signal just over the threshold is compressed, not passed and not slammed to the ceiling.
    SoftLimiter gentle;
    gentle.setThreshold(0.5f);
    const float y = gentle.process(0.7f);
    REQUIRE(y > 0.5f);
    REQUIRE(y < 0.7f);
}

TEST_CASE("SoftLimiter is monotonic and odd-symmetric", "[dsp]")
{
    SoftLimiter limiter;

    float previous = limiter.process(-5.0f);
    for (float x = -5.0f + 0.01f; x <= 5.0f; x += 0.01f)
    {
        const float y = limiter.process(x);
        REQUIRE(y >= previous - 1.0e-6f);
        REQUIRE_THAT(limiter.process(-x), Catch::Matchers::WithinAbs(-y, 1.0e-5));
        previous = y;
    }
}

TEST_CASE("SoftLimiter rejects an out-of-range threshold", "[dsp]")
{
    SoftLimiter limiter;
    limiter.setThreshold(2.0f);
    REQUIRE(limiter.threshold() > 0.0f);
    REQUIRE(limiter.threshold() < 1.0f);
}
