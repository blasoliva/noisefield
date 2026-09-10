#pragma once

#include "model/Preset.h"

#include <juce_core/juce_core.h>

#include <optional>
#include <vector>

namespace noisefield::io
{

/// Reads and writes user presets as `.nfp` (JSON) files under
/// `~/.config/Noisefield/presets/`.
class PresetStore
{
public:
    [[nodiscard]] juce::File directory() const;

    /// Preset names (file stems), sorted.
    [[nodiscard]] std::vector<juce::String> list() const;

    [[nodiscard]] std::optional<model::Preset> load(const juce::String& name) const;

    /// Writes `preset` to `<name>.nfp` (the file stem is derived from `preset.name`).
    bool save(const model::Preset& preset) const;

    bool remove(const juce::String& name) const;
};

} // namespace noisefield::io
