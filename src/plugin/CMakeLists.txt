juce_add_plugin(NoisefieldPlugin
    PRODUCT_NAME "Noisefield"
    COMPANY_NAME "Noisefield"
    BUNDLE_ID "app.noisefield.NoisefieldPlugin"
    PLUGIN_MANUFACTURER_CODE Nsfd
    PLUGIN_CODE Nsf1
    FORMATS VST3 LV2
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Instrument" "Tools"
    LV2URI "urn:noisefield:noisefield")

target_sources(NoisefieldPlugin PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h)

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/PluginProcessor.cpp"
    "${CMAKE_SOURCE_DIR}/src/engine/SignalGraph.cpp"
    TARGET_DIRECTORY NoisefieldPlugin
    PROPERTIES COMPILE_OPTIONS "${NOISEFIELD_WARNING_OPTIONS}")

target_compile_definitions(NoisefieldPlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(NoisefieldPlugin PRIVATE
    noisefield_engine
    juce::juce_audio_utils
    juce::juce_recommended_config_flags)

if(NOISEFIELD_PLUGIN_CLAP)
    include(FetchClapExtensions)
    clap_juce_extensions_plugin(TARGET NoisefieldPlugin
        CLAP_ID "app.noisefield.noisefield"
        CLAP_FEATURES instrument utility "noise")
endif()
