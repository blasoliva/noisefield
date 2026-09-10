#include "io/PresetStore.h"

#include "model/PresetJson.h"

#include <algorithm>

namespace noisefield::io
{

namespace
{
juce::File presetsDir()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Noisefield")
        .getChildFile("presets");
}

juce::String fileStem(const juce::String& name)
{
    return juce::File::createLegalFileName(name.trim());
}
} // namespace

juce::File PresetStore::directory() const
{
    return presetsDir();
}

std::vector<juce::String> PresetStore::list() const
{
    std::vector<juce::String> names;
    for (const auto& file : presetsDir().findChildFiles(juce::File::findFiles, false, "*.nfp"))
        names.push_back(file.getFileNameWithoutExtension());
    std::sort(names.begin(),
              names.end(),
              [](const juce::String& a, const juce::String& b)
              { return a.compareIgnoreCase(b) < 0; });
    return names;
}

std::optional<model::Preset> PresetStore::load(const juce::String& name) const
{
    const auto file = presetsDir().getChildFile(fileStem(name) + ".nfp");
    if (!file.existsAsFile())
        return std::nullopt;

    model::Preset preset;
    if (!model::fromJson(file.loadFileAsString().toStdString(), preset))
        return std::nullopt;

    preset.name = name.toStdString(); // the file stem is authoritative
    return preset;
}

bool PresetStore::save(const model::Preset& preset) const
{
    const auto dir = presetsDir();
    if (!dir.createDirectory())
        return false;

    const auto file = dir.getChildFile(fileStem(juce::String(preset.name)) + ".nfp");
    return file.replaceWithText(juce::String(model::toJson(preset)));
}

bool PresetStore::remove(const juce::String& name) const
{
    return presetsDir().getChildFile(fileStem(name) + ".nfp").deleteFile();
}

} // namespace noisefield::io
