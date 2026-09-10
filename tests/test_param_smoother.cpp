#include "dsp/ParamSmoother.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>

using noisefield::dsp::ParamSmoother;

TEST_CASE("ParamSmoother reaches its target after the ramp length", "[dsp]")
{
    ParamSmoother s;
    s.prepare(48000.0, 0.01); // 480-sample ramp
    s.setCurrentAndTarget(0.0f);
    s.setTarget(1.0f);

    REQUIRE(s.isSmoothing());

    float last = 0.0f;
    for (int i = 0; i < 480; ++i)
    {
        const float v = s.nextValue();
        REQUIRE(v >= last - 1.0e-6f); // monotonic increase
        REQUIRE(v <= 1.0f + 1.0e-6f);
        last = v;
    }

    REQUIRE_FALSE(s.isSmoothing());
    REQUIRE_THAT(s.nextValue(), Catch::Matchers::WithinAbs(1.0, 1.0e-6));
}

TEST_CASE("ParamSmoother takes bounded per-sample steps", "[dsp]")
{
    ParamSmoother s;
    s.prepare(48000.0, 0.02); // 960-sample ramp
    s.setCurrentAndTarget(-10.0f);
    s.setTarget(10.0f);

    const float maxStep = 20.0f / 960.0f;
    float previous = s.current();
    for (int i = 0; i < 2000; ++i)
    {
        const float v = s.nextValue();
        REQUIRE(std::abs(v - previous) <= maxStep + 1.0e-4f);
        previous = v;
    }
    REQUIRE_THAT(previous, Catch::Matchers::WithinAbs(10.0, 1.0e-4));
}

TEST_CASE("ParamSmoother setCurrentAndTarget jumps with no ramp", "[dsp]")
{
    ParamSmoother s;
    s.prepare(48000.0, 0.05);
    s.setCurrentAndTarget(3.0f);

    REQUIRE_FALSE(s.isSmoothing());
    REQUIRE(s.current() == 3.0f);
    REQUIRE(s.nextValue() == 3.0f);
}

TEST_CASE("ParamSmoother with a zero-length ramp is immediate", "[dsp]")
{
    ParamSmoother s;
    s.prepare(48000.0, 0.0);
    s.setCurrentAndTarget(0.0f);
    s.setTarget(0.5f);

    REQUIRE_FALSE(s.isSmoothing());
    REQUIRE(s.nextValue() == 0.5f);
}
