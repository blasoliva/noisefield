#pragma once

namespace noisefield::dsp
{

/// Audible frequency range Noisefield exposes for tone generation.
inline constexpr double kMinFrequencyHz = 20.0;
inline constexpr double kMaxFrequencyHz = 20000.0;

/// Below this level a gain control is treated as silence (-inf dB).
inline constexpr float kMinGainDb = -60.0f;

} // namespace noisefield::dsp
