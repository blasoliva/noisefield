#include "core/BuildInfo.h"

#include <noisefield/Config.h>

namespace noisefield
{

std::string buildInfoString()
{
    return std::string(kProjectName) + ' ' + kFullVersion;
}

} // namespace noisefield
