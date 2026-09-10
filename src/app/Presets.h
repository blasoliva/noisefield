#pragma once

#include "model/Preset.h"

#include <vector>

namespace noisefield::app
{

/// The built-in factory presets shown at the top of the Preset menu. They set every control
/// at once; the resulting values persist, the selection itself does not.
const std::vector<model::Preset>& factoryPresets();

} // namespace noisefield::app
