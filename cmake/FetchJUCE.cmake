# Fetches JUCE at configure time. Pin the tag deliberately; bump it in a dedicated commit.
include_guard(GLOBAL)

include(FetchContent)

set(NOISEFIELD_JUCE_TAG "8.0.15" CACHE STRING "JUCE git tag to build against")

message(STATUS "Noisefield: using JUCE ${NOISEFIELD_JUCE_TAG}")

FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG ${NOISEFIELD_JUCE_TAG}
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE)

FetchContent_MakeAvailable(juce)

# Treat JUCE headers as system headers (CMake >= 3.25) so JUCE's own warnings do not show up
# in our build output or trip -Werror on our code.
foreach(_juce_module IN ITEMS
        juce_core juce_events juce_data_structures juce_graphics
        juce_audio_basics juce_audio_devices juce_audio_formats juce_audio_processors
        juce_audio_utils juce_dsp juce_gui_basics juce_gui_extra)
    if(TARGET ${_juce_module})
        set_target_properties(${_juce_module} PROPERTIES SYSTEM ON)
    endif()
endforeach()
