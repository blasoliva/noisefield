#include "dsp/Gain.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using noisefield::dsp::dbToGain;
using noisefield::dsp::gainToDb;

TEST_CASE("dbToGain maps common decibel values", "[dsp]")
{
    REQUIRE_THAT(dbToGain(0.0f), Catch::Matchers::WithinAbs(1.0, 1.0e-6));
    REQUIRE_THAT(dbToGain(-6.0f), Catch::Matchers::WithinAbs(0.5011872, 1.0e-5));
    REQUIRE_THAT(dbToGain(-20.0f), Catch::Matchers::WithinAbs(0.1, 1.0e-6));
}

TEST_CASE("dbToGain snaps to silence at or below minus infinity", "[dsp]")
{
    REQUIRE(dbToGain(noisefield::dsp::kMinGainDb) == 0.0f);
    REQUIRE(dbToGain(-120.0f) == 0.0f);
}

TEST_CASE("gainToDb is the inverse of dbToGain in range", "[dsp]")
{
    for (float db : { -48.0f, -24.0f, -12.0f, -3.0f, 0.0f })
        REQUIRE_THAT(gainToDb(dbToGain(db)), Catch::Matchers::WithinAbs(db, 1.0e-3));
}
