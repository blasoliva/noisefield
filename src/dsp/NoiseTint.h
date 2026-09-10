#pragma once

#include "dsp/NoiseColour.h"

namespace noisefield::dsp
{

/// Shapes a white-noise stream into one of the `NoiseColour` tints. Feed it white samples
/// (see `WhiteNoise`); it applies the colour's spectral tilt and normalises the result so
/// every colour sits at roughly the same RMS as the white input, which keeps the mixer
/// faders meaningful.
///
/// Header-only, allocation-free, safe on the audio thread. Slopes:
///   White   0 dB/oct     Pink  -3     Brown  -6     Blue  +3     Violet  +6
///   Grey    approximately perceptually flat (crude inverse equal-loudness shelving).
class NoiseTint
{
public:
    void setColour(NoiseColour colour) noexcept
    {
        if (colour != colour_)
        {
            colour_ = colour;
            resetState();
        }
    }

    [[nodiscard]] NoiseColour colour() const noexcept
    {
        return colour_;
    }

    void reset() noexcept
    {
        resetState();
    }

    float process(float white) noexcept
    {
        switch (colour_)
        {
        case NoiseColour::White:
            return white;
        case NoiseColour::Pink:
            return pinkRaw(white) * kPinkGain;
        case NoiseColour::Brown:
            return brown(white);
        case NoiseColour::Blue:
            return blue(white);
        case NoiseColour::Violet:
            return violet(white);
        case NoiseColour::Grey:
            return grey(white);
        }
        return white;
    }

private:
    void resetState() noexcept
    {
        p0_ = p1_ = p2_ = p3_ = p4_ = p5_ = p6_ = 0.0f;
        brownState_ = 0.0f;
        prevWhite_ = 0.0f;
        prevPink_ = 0.0f;
        greyLow_ = 0.0f;
        greyHigh_ = 0.0f;
    }

    // Paul Kellett's refined pink-noise filter (7 one-poles summed), unnormalised.
    float pinkRaw(float w) noexcept
    {
        p0_ = 0.99886f * p0_ + w * 0.0555179f;
        p1_ = 0.99332f * p1_ + w * 0.0750759f;
        p2_ = 0.96900f * p2_ + w * 0.1538520f;
        p3_ = 0.86650f * p3_ + w * 0.3104856f;
        p4_ = 0.55000f * p4_ + w * 0.5329522f;
        p5_ = -0.7616f * p5_ - w * 0.0168980f;
        const float out = p0_ + p1_ + p2_ + p3_ + p4_ + p5_ + p6_ + w * 0.5362f;
        p6_ = w * 0.115926f;
        return out;
    }

    // Leaky integrator: -6 dB/oct, bounded (doesn't random-walk away).
    float brown(float w) noexcept
    {
        brownState_ = (brownState_ + 0.02f * w) / 1.02f;
        return brownState_ * kBrownGain;
    }

    // Differentiated white: +6 dB/oct.
    float violet(float w) noexcept
    {
        const float out = w - prevWhite_;
        prevWhite_ = w;
        return out * kVioletGain;
    }

    // Differentiated pink: +3 dB/oct.
    float blue(float w) noexcept
    {
        const float pink = pinkRaw(w);
        const float out = pink - prevPink_;
        prevPink_ = pink;
        return out * kBlueGain;
    }

    // Crude inverse equal-loudness: boost the extremes, leave the ear-sensitive mid alone,
    // so the *perceived* spectrum comes out roughly flat. Not an ISO 226 inverse.
    float grey(float w) noexcept
    {
        greyLow_ += kGreyLowCoeff * (w - greyLow_);    // ~ below 250 Hz
        greyHigh_ += kGreyHighCoeff * (w - greyHigh_); // ~ below 4 kHz
        const float high = w - greyHigh_;              // ~ above 4 kHz
        return (w + 0.9f * greyLow_ + 0.8f * high) * kGreyGain;
    }

    // Gains chosen so each colour's RMS lands within a few percent of the white input's,
    // measured over a 4 M-sample run (see the calibration note in tests/test_noise_tint.cpp).
    static constexpr float kPinkGain = 0.325f;
    static constexpr float kBrownGain = 10.05f;
    static constexpr float kVioletGain = 0.707f;
    static constexpr float kBlueGain = 0.551f;
    static constexpr float kGreyLowCoeff = 0.033f;
    static constexpr float kGreyHighCoeff = 0.42f;
    static constexpr float kGreyGain = 0.663f;

    NoiseColour colour_ = NoiseColour::White;

    float p0_ = 0.0f, p1_ = 0.0f, p2_ = 0.0f, p3_ = 0.0f, p4_ = 0.0f, p5_ = 0.0f, p6_ = 0.0f;
    float brownState_ = 0.0f;
    float prevWhite_ = 0.0f;
    float prevPink_ = 0.0f;
    float greyLow_ = 0.0f;
    float greyHigh_ = 0.0f;
};

} // namespace noisefield::dsp
