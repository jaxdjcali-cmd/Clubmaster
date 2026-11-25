cmake_minimum_required(VERSION 3.22)
project(ClubMaster VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(JUCE)
juce_add_plugin(ClubMaster
    COMPANY_NAME "JaxDJLatino"
    PLUGIN_MANUFACTURER_CODE JaxD
    PLUGIN_CODE ClbM
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Club Master"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE)
juce_generate_juce_header(ClubMaster)
target_compile_definitions(ClubMaster PRIVATE JUCE_VST3_CAN_REPLACE_VST2=0)
target_sources(ClubMaster PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_link_libraries(ClubMaster PRIVATE juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp juce::juce_gui_basics)
