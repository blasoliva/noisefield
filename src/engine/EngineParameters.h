#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace noisefield::engine
{

/// The signal graph renders a fixed pool of layer slots. The pool is sized once (never grown
/// on the audio thread); the GUI/model owns which slots are in use.
inline constexpr int kMaxLayers = 16;

/// What a layer slot generates.
enum class LayerSource
{
    Oscillator = 0,
    Noise = 1,
};

/// Lock-free parameter block for one layer slot. Every field is an atomic the audio thread
/// only ever loads with `std::memory_order_relaxed`.
///
/// Lifecycle: the GUI configures a slot's `source` / value fields, bumps `epoch`, then sets
/// `active` -- the audio thread hard-resets the voice on the epoch change and fades its gain
/// in. Clearing `active` fades the gain out; the voice keeps running until the ramp reaches
/// zero, then the slot goes idle (skipped, no CPU). Re-using a slot for a different layer is
/// another `epoch` bump.
struct LayerParameters
{
    std::atomic<bool> active{false};        // slot is rendered
    std::atomic<int> source{0};             // LayerSource
    std::atomic<bool> muted{false};         // user mute: fades to 0, voice keeps running
    std::atomic<float> gainDb{-12.0f};      //
    std::atomic<float> frequencyHz{220.0f}; // oscillator
    std::atomic<int> noiseColour{0};        // noise: index into dsp::kNoiseColours
    std::atomic<std::uint64_t> seed{1};     // noise
    std::atomic<std::uint64_t> epoch{0};    // bump to hard-reset the voice
};

/// Lock-free parameter block shared between the GUI (writer) and the audio thread (reader).
/// Every field is a trivially-copyable atomic, so the audio callback only ever does relaxed
/// loads -- no locks, no allocation.
struct EngineParameters
{
    // Transport / master
    std::atomic<bool> playing{false};
    std::atomic<bool> masterMute{false};
    std::atomic<float> masterGainDb{-6.0f};
    std::atomic<bool> limiterEnabled{true};

    // Layer pool
    std::array<LayerParameters, kMaxLayers> layers{};

    LayerParameters& layer(int index) noexcept
    {
        return layers[static_cast<std::size_t>(index)];
    }

    const LayerParameters& layer(int index) const noexcept
    {
        return layers[static_cast<std::size_t>(index)];
    }
};

/// Snapshot of the master output level, produced by the audio thread and consumed by the
/// meter on the GUI thread.
struct MeterSnapshot
{
    float peak = 0.0f;
    float rms = 0.0f;
};

} // namespace noisefield::engine
