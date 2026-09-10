# Fetches Catch2 v3 at configure time (only when NOISEFIELD_BUILD_TESTS is ON).
include_guard(GLOBAL)

include(FetchContent)

set(NOISEFIELD_CATCH2_TAG "v3.7.1" CACHE STRING "Catch2 git tag to build against")

FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG ${NOISEFIELD_CATCH2_TAG}
    GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
