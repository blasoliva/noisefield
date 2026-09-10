#include "dsp/NoiseColour.h"
#include "model/Preset.h"
#include "model/PresetJson.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using noisefield::dsp::NoiseColour;
using noisefield::model::fromJson;
using noisefield::model::Preset;
using noisefield::model::toJson;

TEST_CASE("Preset survives a JSON round trip", "[model]")
{
    Preset original;
    original.name = R"(My "focus" mix)";
    original.toneEnabled = false;
    original.toneFrequencyHz = 432.5;
    original.toneGainDb = -9.5;
    original.noiseEnabled = true;
    original.noiseColour = NoiseColour::Brown;
    original.noiseGainDb = -12.0;
    original.noiseSeed = 9876543210ULL;
    original.masterMute = true;
    original.masterGainDb = -3.0;
    original.limiterEnabled = false;

    Preset restored;
    REQUIRE(fromJson(toJson(original), restored));

    REQUIRE(restored.name == original.name);
    REQUIRE(restored.toneEnabled == original.toneEnabled);
    REQUIRE_THAT(restored.toneFrequencyHz, Catch::Matchers::WithinAbs(432.5, 1e-4));
    REQUIRE_THAT(restored.toneGainDb, Catch::Matchers::WithinAbs(-9.5, 1e-4));
    REQUIRE(restored.noiseEnabled == original.noiseEnabled);
    REQUIRE(restored.noiseColour == NoiseColour::Brown);
    REQUIRE_THAT(restored.noiseGainDb, Catch::Matchers::WithinAbs(-12.0, 1e-4));
    REQUIRE(restored.noiseSeed == 9876543210ULL);
    REQUIRE(restored.masterMute == original.masterMute);
    REQUIRE_THAT(restored.masterGainDb, Catch::Matchers::WithinAbs(-3.0, 1e-4));
    REQUIRE(restored.limiterEnabled == original.limiterEnabled);
}

TEST_CASE("Preset JSON carries the schema version", "[model]")
{
    REQUIRE(toJson(Preset{}).find("\"schemaVersion\": 1") != std::string::npos);
}

TEST_CASE("fromJson keeps defaults for missing keys", "[model]")
{
    Preset p; // defaults
    const double defaultFreq = p.toneFrequencyHz;
    REQUIRE(fromJson(R"({ "name": "sparse", "noiseGainDb": -30 })", p));
    REQUIRE(p.name == "sparse");
    REQUIRE_THAT(p.noiseGainDb, Catch::Matchers::WithinAbs(-30.0, 1e-4));
    REQUIRE_THAT(p.toneFrequencyHz, Catch::Matchers::WithinAbs(defaultFreq, 1e-9));
}

TEST_CASE("fromJson ignores unknown keys and nested structures", "[model]")
{
    Preset p;
    const auto json = R"({
        "schemaVersion": 7,
        "name": "future",
        "toneGainDb": -8,
        "layers": [ { "type": "osc" }, { "type": "noise" } ],
        "modMatrix": { "a": 1 },
        "somethingNew": "ignored"
    })";
    REQUIRE(fromJson(json, p));
    REQUIRE(p.name == "future");
    REQUIRE_THAT(p.toneGainDb, Catch::Matchers::WithinAbs(-8.0, 1e-4));
}

TEST_CASE("fromJson rejects malformed input", "[model]")
{
    Preset p;
    REQUIRE_FALSE(fromJson("not json", p));
    REQUIRE_FALSE(fromJson(R"({ "name": )", p));
    REQUIRE_FALSE(fromJson(R"({ "name": "x" "toneGainDb": 1 })", p)); // missing comma
    REQUIRE_FALSE(fromJson("", p));
}

TEST_CASE("fromJson tolerates an unknown noise colour", "[model]")
{
    Preset p;
    REQUIRE(fromJson(R"({ "noiseColour": "Chartreuse" })", p));
    REQUIRE(p.noiseColour == NoiseColour::White);
}
