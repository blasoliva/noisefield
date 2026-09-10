#pragma once

#include <atomic>
#include <cstdint>

namespace noisefield::engine
{

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

    // Tone source
    std::atomic<bool> toneEnabled{true};
    std::atomic<float> toneFrequencyHz{220.0f};
    std::atomic<float> toneGainDb{-14.0f};

    // Noise source
    std::atomic<bool> noiseEnabled{false};
    std::atomic<float> noiseGainDb{-20.0f};
    std::atomic<std::uint64_t> noiseSeed{1};
};

/// Snapshot of the master output level, produced by the audio thread and consumed by the
/// meter on the GUI thread.
struct MeterSnapshot
{
    float peak = 0.0f;
    float rms = 0.0f;
};

} // namespace noisefield::engine
