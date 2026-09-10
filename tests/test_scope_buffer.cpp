#include "dsp/ScopeBuffer.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

using noisefield::dsp::ScopeBuffer;

TEST_CASE("ScopeBuffer readLatest returns the most recent samples in order", "[dsp]")
{
    ScopeBuffer scope;
    std::array<float, 100> in{};
    for (int i = 0; i < 100; ++i)
        in[static_cast<size_t>(i)] = static_cast<float>(i);
    scope.write(in.data(), 100);

    std::array<float, 10> out{};
    scope.readLatest(out.data(), 10);
    for (int i = 0; i < 10; ++i)
        REQUIRE(out[static_cast<size_t>(i)] == static_cast<float>(90 + i));

    REQUIRE(scope.samplesWritten() == 100);
}

TEST_CASE("ScopeBuffer zero-fills reads that predate any write", "[dsp]")
{
    ScopeBuffer scope;
    const std::array<float, 3> in{1.0f, 2.0f, 3.0f};
    scope.write(in.data(), 3);

    std::array<float, 8> out{};
    scope.readLatest(out.data(), 8);
    REQUIRE(out[0] == 0.0f);
    REQUIRE(out[4] == 0.0f);
    REQUIRE(out[5] == 1.0f);
    REQUIRE(out[6] == 2.0f);
    REQUIRE(out[7] == 3.0f);
}

TEST_CASE("ScopeBuffer keeps the newest window after wrapping", "[dsp]")
{
    ScopeBuffer scope;
    const int total = ScopeBuffer::kCapacity * 3 + 777;
    std::vector<float> chunk(256, 0.0f);
    int written = 0;
    while (written < total)
    {
        const int n = std::min(256, total - written);
        for (int i = 0; i < n; ++i)
            chunk[static_cast<size_t>(i)] = static_cast<float>(written + i);
        scope.write(chunk.data(), n);
        written += n;
    }

    std::array<float, 512> out{};
    scope.readLatest(out.data(), 512);
    for (int i = 0; i < 512; ++i)
        REQUIRE(out[static_cast<size_t>(i)] == static_cast<float>(total - 512 + i));

    REQUIRE(scope.samplesWritten() == static_cast<std::uint64_t>(total));
}

TEST_CASE("ScopeBuffer reset clears history", "[dsp]")
{
    ScopeBuffer scope;
    const std::array<float, 4> in{5.0f, 6.0f, 7.0f, 8.0f};
    scope.write(in.data(), 4);
    scope.reset();

    REQUIRE(scope.samplesWritten() == 0);
    std::array<float, 4> out{9.0f, 9.0f, 9.0f, 9.0f};
    scope.readLatest(out.data(), 4);
    for (float v : out)
        REQUIRE(v == 0.0f);
}
