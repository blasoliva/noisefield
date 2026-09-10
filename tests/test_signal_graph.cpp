#include "engine/SignalGraph.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

using noisefield::engine::kMaxLayers;
using noisefield::engine::LayerSource;
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

// Largest sample-to-sample jump -- a proxy for an audible click / step discontinuity.
float maxStep(const std::vector<float>& x)
{
    float worst = 0.0f;
    for (size_t i = 1; i < x.size(); ++i)
        worst = std::max(worst, std::abs(x[i] - x[i - 1]));
    return worst;
}

double windowRms(const std::vector<float>& x, size_t begin, size_t end)
{
    double sum = 0.0;
    for (size_t i = begin; i < end && i < x.size(); ++i)
        sum += static_cast<double>(x[i]) * static_cast<double>(x[i]);
    return std::sqrt(sum / static_cast<double>(end - begin));
}

// Configures slot `s` as an oscillator at `hz` and marks it active. `prepare()` must already
// have run.
void addOscillator(SignalGraph& graph, int s, float hz, float gainDb)
{
    auto& l = graph.parameters().layer(s);
    l.source.store(static_cast<int>(LayerSource::Oscillator));
    l.frequencyHz.store(hz);
    l.gainDb.store(gainDb);
    l.muted.store(false);
    l.epoch.fetch_add(1);
    l.active.store(true);
}

void addNoise(SignalGraph& graph, int s, int colour, float gainDb, std::uint64_t seed = 1)
{
    auto& l = graph.parameters().layer(s);
    l.source.store(static_cast<int>(LayerSource::Noise));
    l.noiseColour.store(colour);
    l.seed.store(seed);
    l.gainDb.store(gainDb);
    l.muted.store(false);
    l.epoch.fetch_add(1);
    l.active.store(true);
}
} // namespace

TEST_CASE("SignalGraph is silent while not playing", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    addOscillator(graph, 0, 440.0f, 0.0f);
    graph.parameters().playing.store(false);

    const auto out = render(graph, 20);
    REQUIRE(out.peak == 0.0f);
}

TEST_CASE("SignalGraph renders a tone at the expected level on both channels", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    graph.parameters().playing.store(true);
    graph.parameters().masterGainDb.store(0.0f);
    graph.parameters().limiterEnabled.store(false);
    addOscillator(graph, 0, 1000.0f, -6.0f); // ~0.5 linear

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
    graph.parameters().playing.store(true);
    addOscillator(graph, 0, 440.0f, 0.0f);
    graph.parameters().masterMute.store(true);

    const auto out = render(graph, 50);
    REQUIRE(out.peak < 1.0e-4f);
}

TEST_CASE("SignalGraph limiter keeps the output inside the unit interval", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(true);
    addOscillator(graph, 0, 440.0f, 0.0f);
    addNoise(graph, 1, 0, 0.0f);

    const auto out = render(graph, 100);
    REQUIRE(out.peak <= 1.0f);
}

TEST_CASE("SignalGraph feeds the meter and the scope", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    graph.parameters().playing.store(true);
    graph.parameters().masterGainDb.store(0.0f);
    addOscillator(graph, 0, 440.0f, -6.0f);

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
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(true);
    addNoise(graph, 0, 0, -6.0f);

    for (int colour = 0; colour < 6; ++colour)
    {
        p.layer(0).noiseColour.store(colour);
        const auto out = render(graph, 20);
        REQUIRE(std::isfinite(out.leftRms));
        REQUIRE(out.peak <= 1.0f);
        REQUIRE(out.leftRms > 0.0);
    }
}

TEST_CASE("SignalGraph sums independent layers", "[engine]")
{
    auto rmsOf = [](auto configure)
    {
        SignalGraph graph;
        graph.prepare(kSampleRate, kBlock);
        auto& p = graph.parameters();
        p.playing.store(true);
        p.masterGainDb.store(0.0f);
        p.limiterEnabled.store(false);
        configure(graph);
        return render(graph, 200).leftRms;
    };

    const double toneOnly = rmsOf([](SignalGraph& g) { addOscillator(g, 0, 300.0f, -12.0f); });
    const double noiseOnly = rmsOf([](SignalGraph& g) { addNoise(g, 1, 1, -12.0f); });
    const double both = rmsOf(
        [](SignalGraph& g)
        {
            addOscillator(g, 0, 300.0f, -12.0f);
            addNoise(g, 1, 1, -12.0f);
        });

    // Uncorrelated sources: powers add, so the combined RMS is ~sqrt(a^2 + b^2).
    const double expected = std::sqrt(toneOnly * toneOnly + noiseOnly * noiseOnly);
    REQUIRE(toneOnly > 0.0);
    REQUIRE(noiseOnly > 0.0);
    REQUIRE_THAT(both, Catch::Matchers::WithinRel(expected, 0.1));
}

TEST_CASE("SignalGraph fades a layer out without a click", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(false);
    addOscillator(graph, 0, 500.0f, -6.0f);

    render(graph, 100); // settle
    p.layer(0).active.store(false);
    const auto out = render(graph, 100); // ~1 s, well past the fade

    const size_t total = out.left.size();
    REQUIRE(windowRms(out.left, 0, total / 8) > 0.05);       // still ringing at the start
    REQUIRE(windowRms(out.left, total / 2, total) < 1.0e-4); // silent by the end
    REQUIRE(maxStep(out.left) < 0.05f);                      // no step discontinuity
}

TEST_CASE("SignalGraph fades a layer in from silence", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(false);
    addOscillator(graph, 0, 500.0f, -6.0f);

    const auto out = render(graph, 100);

    REQUIRE(std::abs(out.left.front()) < 1.0e-3f); // starts at silence
    const size_t total = out.left.size();
    REQUIRE(windowRms(out.left, 0, 64) < windowRms(out.left, total - 64, total)); // ramps up
    REQUIRE(maxStep(out.left) < 0.05f);
}

TEST_CASE("SignalGraph crossfades one layer into another with no gap", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(false);
    addOscillator(graph, 0, 400.0f, -6.0f);

    render(graph, 100); // settle A

    // Swap A for B in the same instant.
    p.layer(0).active.store(false);
    addOscillator(graph, 1, 700.0f, -6.0f);
    const auto out = render(graph, 100);

    // Every 5 ms window keeps some signal -- the mix never drops out during the swap.
    const size_t win = static_cast<size_t>(kSampleRate * 0.005);
    for (size_t begin = 0; begin + win < out.left.size(); begin += win)
        REQUIRE(windowRms(out.left, begin, begin + win) > 0.05);
    REQUIRE(maxStep(out.left) < 0.08f);
}

TEST_CASE("SignalGraph reuses a slot for a different layer via an epoch bump", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(true);

    addOscillator(graph, 0, 1000.0f, -6.0f);
    render(graph, 100);
    p.layer(0).active.store(false);
    render(graph, 100); // let the tone fade fully

    // Same slot, now a brown-noise layer with a fresh seed.
    addNoise(graph, 0, 2, -6.0f, 0x1234ABCDu);
    const auto out = render(graph, 200);

    REQUIRE(std::isfinite(out.leftRms));
    REQUIRE(out.leftRms > 0.0);
    REQUIRE(out.peak <= 1.0f);
    for (const float s : out.left)
        REQUIRE(std::isfinite(s));
}

TEST_CASE("SignalGraph keeps a muted but active layer silent", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.masterGainDb.store(0.0f);
    addOscillator(graph, 0, 440.0f, 0.0f);
    p.layer(0).muted.store(true);

    const auto out = render(graph, 80);
    REQUIRE(out.peak < 1.0e-4f);
}

TEST_CASE("SignalGraph stays bounded with every layer slot active", "[engine]")
{
    SignalGraph graph;
    graph.prepare(kSampleRate, kBlock);
    auto& p = graph.parameters();
    p.playing.store(true);
    p.masterGainDb.store(0.0f);
    p.limiterEnabled.store(true);

    for (int s = 0; s < kMaxLayers; ++s)
    {
        if (s % 2 == 0)
            addOscillator(graph, s, 100.0f + 50.0f * static_cast<float>(s), -20.0f);
        else
            addNoise(graph, s, s % 6, -20.0f, static_cast<std::uint64_t>(s) * 2654435761u + 1u);
    }

    const auto out = render(graph, 100);
    REQUIRE(std::isfinite(out.leftRms));
    REQUIRE(out.leftRms > 0.0);
    REQUIRE(out.peak <= 1.0f);
}
