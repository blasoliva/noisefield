# --------------------------------------------------------------------------------------------
# Generated build-configuration header
# --------------------------------------------------------------------------------------------
configure_file(NoisefieldConfig.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/generated/noisefield/Config.h" @ONLY)

# Warning flags carried by the noisefield_warnings INTERFACE target, reused below as
# per-source-file options so they apply to our code but never to JUCE's unity .cpp files.
get_target_property(NOISEFIELD_WARNING_OPTIONS noisefield_warnings INTERFACE_COMPILE_OPTIONS)
if(NOT NOISEFIELD_WARNING_OPTIONS)
    set(NOISEFIELD_WARNING_OPTIONS "")
endif()

# --------------------------------------------------------------------------------------------
# noisefield_dsp -- header-only DSP + data model. No JUCE; this is what unit tests build on.
# --------------------------------------------------------------------------------------------
add_library(noisefield_dsp INTERFACE)
target_include_directories(noisefield_dsp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(noisefield_dsp INTERFACE
    dsp/Constants.h
    dsp/Gain.h
    dsp/LevelDetector.h
    dsp/NoiseColour.h
    dsp/ParamSmoother.h
    dsp/SineOscillator.h
    dsp/SoftLimiter.h
    dsp/WhiteNoise.h
    model/Layer.h
    model/Project.h)

# --------------------------------------------------------------------------------------------
# noisefield_core -- small compiled pieces with no JUCE dependency (build metadata, ...).
# --------------------------------------------------------------------------------------------
add_library(noisefield_core STATIC
    core/BuildInfo.cpp
    core/BuildInfo.h)
target_include_directories(noisefield_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(noisefield_core
    PUBLIC noisefield_dsp
    PRIVATE noisefield_warnings)

# Placeholders for later milestones -- kept so the directories and intent are visible from M1.
# gui/  -> M3 components (layer list, mixer strip, analyzer view, transport bar)
# io/   -> M4 recorder, offline exporter, headless CLI

# --------------------------------------------------------------------------------------------
# Noisefield -- the standalone GUI application. This is the single target that compiles the
# JUCE modules, so JUCE's own warnings must not be forced on here; our sources get the strict
# flags individually instead.
# --------------------------------------------------------------------------------------------
juce_add_gui_app(Noisefield
    PRODUCT_NAME "Noisefield"
    VERSION "${PROJECT_VERSION}"
    COMPANY_NAME "Noisefield"
    BUNDLE_ID "app.noisefield.Noisefield"
    ICON_BIG "${CMAKE_SOURCE_DIR}/resources/icons/noisefield-512.png"
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE)

# Bundled resources (the in-app user guide).
juce_add_binary_data(noisefield_assets
    HEADER_NAME BinaryData.h
    NAMESPACE BinaryData
    SOURCES "${CMAKE_SOURCE_DIR}/docs/guide.md")

set(NOISEFIELD_APP_SOURCES
    app/Main.cpp
    app/MainComponent.cpp
    engine/AudioEngine.cpp
    gui/LevelMeter.cpp
    gui/SettingsComponent.cpp
    gui/GuideView.cpp)

target_sources(Noisefield PRIVATE
    ${NOISEFIELD_APP_SOURCES}
    app/MainComponent.h
    app/NoisefieldLookAndFeel.h
    engine/AudioEngine.h
    engine/EngineParameters.h
    gui/DetachedWindow.h
    gui/LevelMeter.h
    gui/SettingsComponent.h
    gui/GuideView.h)

set_source_files_properties(${NOISEFIELD_APP_SOURCES}
    TARGET_DIRECTORY Noisefield
    PROPERTIES COMPILE_OPTIONS "${NOISEFIELD_WARNING_OPTIONS}")

target_include_directories(Noisefield PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(Noisefield PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    JUCE_APPLICATION_NAME_STRING="Noisefield"
    JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}"
    JUCE_DISPLAY_SPLASH_SCREEN=0
    $<$<CONFIG:Debug>:JUCE_DEBUG=1>)

target_link_libraries(Noisefield PRIVATE
    noisefield_core
    noisefield_assets
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags)
