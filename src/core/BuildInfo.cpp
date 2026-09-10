#include "core/BuildInfo.h"

#include <noisefield/Config.h>

namespace noisefield
{

std::string buildInfoString()
{
    return std::string(kProjectName) + ' ' + kVersionString;
}

} // namespace noisefield
