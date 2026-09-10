#pragma once

#include "model/Layer.h"

#include <string>
#include <vector>

namespace noisefield::model
{

/// Master-bus settings.
struct MasterBus
{
    float gainDb = 0.0f;
    bool mute = false;
    bool limiterEnabled = true;
};

/// Session settings (timer, fades). Wired up in milestone M4.
struct SessionSettings
{
    bool timerEnabled = false;
    double durationSeconds = 600.0;
    double fadeInSeconds = 2.0;
    double fadeOutSeconds = 5.0;
};

/// The full document Noisefield loads, edits and saves. JSON (de)serialization with a schema
/// version is milestone M3, task NF-040.
struct Project
{
    static constexpr int kSchemaVersion = 1;

    std::string name = "Untitled";
    std::vector<Layer> layers{};
    MasterBus master{};
    SessionSettings session{};
};

} // namespace noisefield::model
