cmake_minimum_required(VERSION 3.22)

project(Noisefield
    VERSION 0.3.0 # x-release-please-version
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

# Full version string for packaging and the About text. Defaults to the numeric project
# version; the release workflow overrides it with the tag (e.g. 0.1.0-alpha.1).
set(NOISEFIELD_VERSION "${PROJECT_VERSION}" CACHE STRING "Full version string")

option(NOISEFIELD_BUILD_TESTS "Build the Catch2 test suite" OFF)
option(NOISEFIELD_BUILD_PLUGIN "Build the VST3/LV2 plugin" OFF)
option(NOISEFIELD_PLUGIN_CLAP "Also build a CLAP (fetches clap-juce-extensions)" ON)
option(NOISEFIELD_WERROR "Treat compiler warnings as errors" OFF)

# Explicit JACK backend (NF-073): only compiled into the standalone app when libjack's headers
# are available, so an ALSA-only machine still builds fine (see docs/building.md).
find_path(NOISEFIELD_JACK_INCLUDE_DIR NAMES jack/jack.h)
if(NOISEFIELD_JACK_INCLUDE_DIR)
    message(STATUS "Noisefield: libjack headers found -- JACK backend enabled (NF-073)")
else()
    message(STATUS "Noisefield: libjack headers not found -- JACK backend disabled "
                    "(install libjack-jackd2-dev / jack-audio-connection-kit-devel to enable it)")
endif()

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

# --------------------------------------------------------------------------------------------
# Packaging (CPack). `cpack -G DEB` in the build dir produces noisefield_<version>_amd64.deb.
# --------------------------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "noisefield")
set(CPACK_PACKAGE_VENDOR "Noisefield")
set(CPACK_PACKAGE_VERSION "${NOISEFIELD_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/blasoliva/noisefield")
set(CPACK_PACKAGE_CONTACT "Blas Oliva <blasoliva@gmail.com>")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_STRIP_FILES ON)

# Package only our own install component (not JUCE's install rules), all in one archive.
set(CPACK_COMPONENTS_ALL noisefield)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)

set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # derive Depends from the linked libraries
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")

include(CPack)
