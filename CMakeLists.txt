cmake_minimum_required(VERSION 3.22)

project(Noisefield
    VERSION 0.1.0
    DESCRIPTION "Tone generator and noise mixer for Linux"
    LANGUAGES C CXX)

# --------------------------------------------------------------------------------------------
# Global settings
# --------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(NOISEFIELD_BUILD_TESTS "Build the Catch2 test suite" OFF)
option(NOISEFIELD_WERROR "Treat compiler warnings as errors" OFF)

# --------------------------------------------------------------------------------------------
# Shared warning flags (INTERFACE target applied to our own code, not to JUCE/deps)
# --------------------------------------------------------------------------------------------
add_library(noisefield_warnings INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(noisefield_warnings INTERFACE
        -Wall -Wextra -Wpedantic
        -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual
        -Wnull-dereference -Wdouble-promotion -Wformat=2
        -Wconversion -Wsign-conversion)
    if(NOISEFIELD_WERROR)
        target_compile_options(noisefield_warnings INTERFACE -Werror)
    endif()
endif()

# --------------------------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------------------------
include(FetchJUCE)

# --------------------------------------------------------------------------------------------
# Targets
# --------------------------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(tools)

if(NOISEFIELD_BUILD_TESTS)
    enable_testing()
    include(FetchCatch2)
    add_subdirectory(tests)
endif()
