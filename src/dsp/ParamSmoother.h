#pragma once

namespace noisefield::dsp
{

/// Linear ramp smoother for a single control value. Setting a new target starts a ramp that
/// reaches it after `rampLengthSeconds`; `nextValue()` advances the ramp by one sample.
///
/// Header-only and free of any framework dependency so it is trivially unit-testable and
/// usable from the real-time thread (no allocation, no locks).
class ParamSmoother
{
public:
    void prepare(double sampleRate, double rampLengthSeconds) noexcept
    {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
        rampLengthSeconds_ = rampLengthSeconds > 0.0 ? rampLengthSeconds : 0.0;
        rampSamples_ = static_cast<int>(sampleRate_ * rampLengthSeconds_);
        setCurrentAndTarget(target_);
    }

    /// Jump straight to `value` with no ramp.
    void setCurrentAndTarget(float value) noexcept
    {
        current_ = value;
        target_ = value;
        step_ = 0.0f;
        countdown_ = 0;
    }

    void setTarget(float value) noexcept
    {
        if (value == target_)
            return;

        target_ = value;

        if (rampSamples_ <= 0)
        {
            current_ = value;
            step_ = 0.0f;
            countdown_ = 0;
            return;
        }

        countdown_ = rampSamples_;
        step_ = (target_ - current_) / static_cast<float>(rampSamples_);
    }

    float nextValue() noexcept
    {
        if (countdown_ <= 0)
            return current_ = target_;

        current_ += step_;
        if (--countdown_ == 0)
            current_ = target_;
        return current_;
    }

    [[nodiscard]] float current() const noexcept { return current_; }
    [[nodiscard]] float target() const noexcept { return target_; }
    [[nodiscard]] bool isSmoothing() const noexcept { return countdown_ > 0; }

private:
    double sampleRate_ = 44100.0;
    double rampLengthSeconds_ = 0.0;
    int rampSamples_ = 0;
    int countdown_ = 0;
    float current_ = 0.0f;
    float target_ = 0.0f;
    float step_ = 0.0f;
};

} // namespace noisefield::dsp
