#include "core/BuildInfo.h"

#include <noisefield/Config.h>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("buildInfoString reports project name and version", "[core]")
{
    const std::string info = noisefield::buildInfoString();

    REQUIRE(info.find("Noisefield") != std::string::npos);
    REQUIRE(info.find(noisefield::kVersionString) != std::string::npos);
    REQUIRE(noisefield::kVersionMajor >= 0);
}
