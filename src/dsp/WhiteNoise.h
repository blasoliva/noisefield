#pragma once

#include <cstdint>

namespace noisefield::dsp
{

/// xoshiro256++ PRNG -- fast, good statistical quality, trivial to seed per layer.
/// Reference: https://prng.di.unimi.it/
class Xoshiro256pp
{
public:
    explicit Xoshiro256pp(std::uint64_t seed = 0x9E3779B97F4A7C15ULL) noexcept
    {
        // SplitMix64 to expand a single seed into the 256-bit state.
        for (auto& word : state_)
        {
            seed += 0x9E3779B97F4A7C15ULL;
            std::uint64_t z = seed;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            word = z ^ (z >> 31);
        }
    }

    std::uint64_t nextUInt64() noexcept
    {
        const std::uint64_t result = rotl(state_[0] + state_[3], 23) + state_[0];
        const std::uint64_t t = state_[1] << 17;

        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];
        state_[2] ^= t;
        state_[3] = rotl(state_[3], 45);

        return result;
    }

    /// Uniform float in [-1, 1).
    float nextBipolarFloat() noexcept
    {
        // Top 24 bits -> [0, 1), then map to [-1, 1).
        const auto unipolar = static_cast<float>(nextUInt64() >> 40) * (1.0f / 16777216.0f);
        return unipolar * 2.0f - 1.0f;
    }

private:
    static std::uint64_t rotl(std::uint64_t x, int k) noexcept
    {
        return (x << k) | (x >> (64 - k));
    }

    std::uint64_t state_[4]{};
};

/// White noise generator (flat power spectral density). Other noise colours (pink, brown,
/// blue, violet, grey) are added in milestone M3, layered on top of this source.
class WhiteNoise
{
public:
    explicit WhiteNoise(std::uint64_t seed = 1) noexcept : rng_(seed) {}

    void setSeed(std::uint64_t seed) noexcept
    {
        rng_ = Xoshiro256pp(seed);
    }

    float nextSample() noexcept
    {
        return rng_.nextBipolarFloat();
    }

private:
    Xoshiro256pp rng_;
};

} // namespace noisefield::dsp
