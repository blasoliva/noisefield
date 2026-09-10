#pragma once

#include "model/Preset.h"

#include <string>
#include <string_view>

namespace noisefield::model
{

/// Serialises a Preset to a small, flat JSON object (the on-disk `.nfp` format), with a
/// `schemaVersion` field.
std::string toJson(const Preset& preset);

/// Parses `toJson` output back into `out`. Returns false only for malformed JSON; unknown
/// keys are ignored and missing keys keep whatever `out` already holds (pass a
/// default-constructed Preset for sensible fallbacks). A newer `schemaVersion` is accepted
/// best-effort.
bool fromJson(std::string_view json, Preset& out);

} // namespace noisefield::model
