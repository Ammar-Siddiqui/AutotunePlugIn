cmake_minimum_required(VERSION 3.22)

project(AutotunePlugin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
)

FetchContent_MakeAvailable(juce)

juce_add_plugin(AutotunePlugin
    COMPANY_NAME "Ammar"
    BUNDLE_ID com.ammar.autotuneplugin
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Ammr
    PLUGIN_CODE Atn1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Autotune Plugin"
)

target_sources(AutotunePlugin
    PRIVATE
        PluginProcessor.cpp
        PluginProcessor.h
        PluginEditor.h
        YinPitchDetector.cpp
        YinPitchDetector.h
)

target_compile_definitions(AutotunePlugin
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(AutotunePlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
