#include "dsp/LevelDetector.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <array>
#include <cmath>
#include <numbers>

using noisefield::dsp::measureBlock;

TEST_CASE("measureBlock reports peak and RMS of a sine block", "[dsp]")
{
    std::array<float, 4800> block{};
    for (size_t i = 0; i < block.size(); ++i)
        block[i] = 0.5f * std::sin(2.0f * std::numbers::pi_v<float> * 100.0f *
                                   (static_cast<float>(i) / 48000.0f));

    const auto level = measureBlock(block.data(), static_cast<int>(block.size()));

    REQUIRE_THAT(level.peak, Catch::Matchers::WithinAbs(0.5, 0.01));
    REQUIRE_THAT(level.rms, Catch::Matchers::WithinAbs(0.5 / std::numbers::sqrt2, 0.01));
}

TEST_CASE("measureBlock handles empty and null input", "[dsp]")
{
    REQUIRE(measureBlock(nullptr, 128).peak == 0.0f);

    std::array<float, 8> block{};
    const auto level = measureBlock(block.data(), 0);
    REQUIRE(level.peak == 0.0f);
    REQUIRE(level.rms == 0.0f);
}
