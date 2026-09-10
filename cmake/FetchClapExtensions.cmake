# Fetches clap-juce-extensions (which pulls the CLAP SDK + helpers as submodules) so the
# plugin can also build a CLAP. Only pulled in when NOISEFIELD_PLUGIN_CLAP is ON.
include_guard(GLOBAL)

include(FetchContent)

set(NOISEFIELD_CLAP_EXT_TAG "9fbefae3d9c3d130aafb558c1ec15427a4bd24be"
    CACHE STRING "clap-juce-extensions git ref")

FetchContent_Declare(clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG ${NOISEFIELD_CLAP_EXT_TAG}
    GIT_SUBMODULES_RECURSE TRUE
    GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(clap-juce-extensions)
