#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace noisefield::dsp
{

/// Single-producer / single-consumer ring of recent output samples, feeding the oscilloscope.
///
/// The audio thread calls `write()`; the GUI thread calls `readLatest()`. Lock-free and
/// allocation-free. A very slow reader can see a torn sample or two at the far (oldest) end of
/// its window while the writer laps it — harmless for a scope, and the recent samples it
/// actually draws are always coherent.
class ScopeBuffer
{
public:
    static constexpr int kCapacity = 8192;

    void write(const float* data, int numSamples) noexcept
    {
        std::uint64_t pos = writePos_;
        for (int i = 0; i < numSamples; ++i)
            buffer_[static_cast<std::size_t>(pos++) & kMask] = data[i];
        writePos_ = pos;
        published_.store(pos, std::memory_order_release);
    }

    void reset() noexcept
    {
        buffer_.fill(0.0f);
        writePos_ = 0;
        published_.store(0, std::memory_order_release);
    }

    /// Copies the most recent `count` samples (chronological order) into `dst`. Slots that
    /// predate any write are filled with 0. `count` must be <= kCapacity.
    void readLatest(float* dst, int count) noexcept
    {
        const std::uint64_t end = published_.load(std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
        {
            const std::int64_t idx = static_cast<std::int64_t>(end) - count + i;
            dst[i] = idx < 0 ? 0.0f : buffer_[static_cast<std::size_t>(idx) & kMask];
        }
    }

    [[nodiscard]] std::uint64_t samplesWritten() const noexcept
    {
        return published_.load(std::memory_order_acquire);
    }

private:
    static constexpr std::size_t kMask = static_cast<std::size_t>(kCapacity) - 1;
    static_assert((kCapacity & (kCapacity - 1)) == 0, "kCapacity must be a power of two");

    std::array<float, kCapacity> buffer_{};
    std::uint64_t writePos_ = 0; // audio thread only
    std::atomic<std::uint64_t> published_{0};
};

} // namespace noisefield::dsp
