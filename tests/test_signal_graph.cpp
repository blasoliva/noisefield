#include "engine/SignalGraph.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

using noisefield::engine::SignalGraph;

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlock = 512;

struct Rendered
{
    std::vector<float> left;
    std::vector<float> right;
    double leftRms = 0.0;
    float peak = 0.0f;
};

Rendered render(SignalGraph& graph, int numBlocks)
{
    Rendered r;
    std::array<float, kBlock> left{};
    std::array<float, kBlock> right{};
    std::array<float*, 2> channels{left.data(), right.data()};

    double sumSquares = 0.0;
    for (int b = 0; b < numBlocks; ++b)
    {
        left.fill(0.0f);
        right.fill(0.0f);
        graph.process(channels.data(), 2, kBlock);
        for (int i = 0; i < kBlock; ++i)
        {
            const float sample = left[static_cast<size_t>(i)];
            r.left.push_back(sample);
            r.right.push_back(right[static_cast<size_t>(i)]);
            sumSquares += static_cast<double>(sample) * static_cast<double>(sample);
            r.peak = std::max(r.peak, std::abs(sample));
        }
    }
    r.leftRms = std::sqrt(sumSquares / static_cast<double>(numBlocks * kBlock));
    return r;
}
} // namespace

TEST_CASE("SignalGraph is silent while not playing", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    graph.parameters().playing.store(false);
    graph.parameters().toneEnabled.store(true);
    graph.parameters().toneGainDb.store(0.0f);

    const auto out = render(graph, 20);
    REQUIRE(out.peak == 0.0f);
}

TEST_CASE("SignalGraph renders a tone at the expected level on both channels", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.toneEnabled.store(true);
    p.toneFrequencyHz.store(1000.0f);
    p.toneGainDb.store(-6.0f); // ~0.5 linear
    p.noiseEnabled.store(false);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(false);

    const auto out = render(graph, 200); // ~2.1 s, past all ramps

    // full-scale sine RMS is 1/sqrt(2); at -6 dB that is ~0.354.
    REQUIRE_THAT(out.leftRms, Catch::Matchers::WithinAbs(0.3536, 0.03));
    for (size_t i = 0; i < out.left.size(); ++i)
        REQUIRE(out.left[i] == out.right[i]);
}

TEST_CASE("SignalGraph master mute silences the output", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.toneEnabled.store(true);
    p.toneGainDb.store(0.0f);
    p.masterMute.store(true);

    const auto out = render(graph, 50);
    REQUIRE(out.peak < 1.0e-4f);
}

TEST_CASE("SignalGraph limiter keeps the output inside the unit interval", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.toneEnabled.store(true);
    p.toneGainDb.store(0.0f);
    p.noiseEnabled.store(true);
    p.noiseGainDb.store(0.0f);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(true);

    const auto out = render(graph, 100);
    REQUIRE(out.peak <= 1.0f);
}

TEST_CASE("SignalGraph feeds the meter and the scope", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.toneEnabled.store(true);
    p.toneGainDb.store(-6.0f);
    p.masterGainDb.store(0.0f);

    render(graph, 100);

    const auto meter = graph.fetchMeterAndReset();
    REQUIRE(meter.rms > 0.2f);
    REQUIRE(meter.peak > 0.3f);

    std::array<float, 1024> scope{};
    graph.readScope(scope.data(), 1024);
    float scopePeak = 0.0f;
    for (const float s : scope)
        scopePeak = std::max(scopePeak, std::abs(s));
    REQUIRE(scopePeak > 0.3f);
}

TEST_CASE("SignalGraph noise colour switch stays bounded and finite", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.toneEnabled.store(false);
    p.noiseEnabled.store(true);
    p.noiseGainDb.store(-6.0f);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(true);

    for (int colour = 0; colour < 6; ++colour)
    {
        p.noiseColour.store(colour);
        const auto out = render(graph, 20);
        REQUIRE(std::isfinite(out.leftRms));
        REQUIRE(out.peak <= 1.0f);
        REQUIRE(out.leftRms > 0.0);
    }
}
