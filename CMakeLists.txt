cmake_minimum_required(VERSION 3.15)
project(QuackerVSTTests VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure we're building from the correct directory
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/JUCE/CMakeLists.txt")
    message(FATAL_ERROR "JUCE submodule not found. Please run 'git submodule update --init --recursive'")
endif()

# Configure JUCE to only build what we need
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "Don't build JUCE extras")
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Don't build JUCE examples")
set(JUCE_MODULES_ONLY OFF CACHE BOOL "We need more than just modules")

# Add JUCE with minimal configuration
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/JUCE juce_build)

# Create a JUCE console app for tests
juce_add_console_app(QuackerVSTTests
    PRODUCT_NAME "QuackerVST Tests"
    COMPANY_NAME "Acedia Audio"
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE)

# Test source files
set(TEST_SOURCES
    Tests/TestRunner.cpp
    Tests/TremoloLFOTests.cpp
    Tests/PresetManagerTests.cpp
    Tests/PluginProcessorTests.cpp
)

# Plugin source files (core functionality only, no UI for tests)
set(PLUGIN_SOURCES
    QuackerVST/Source/PluginProcessor.cpp
    QuackerVST/Source/TremoloLFO.cpp
    QuackerVST/Source/PresetManager.cpp
    QuackerVST/Source/WaveshapeLFO.cpp
    QuackerVST/Source/PerlinNoise.cpp
    QuackerVST/Source/Presets.cpp
)

# UI components (only if they exist and are needed for compilation)
set(UI_SOURCES "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/Source/HierarchicalPresetMenu.cpp")
    list(APPEND UI_SOURCES QuackerVST/Source/HierarchicalPresetMenu.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/Source/Fonts/FontManager.cpp")
    list(APPEND UI_SOURCES QuackerVST/Source/Fonts/FontManager.cpp)
endif()

# Add all sources to the target
target_sources(QuackerVSTTests PRIVATE
    ${TEST_SOURCES}
    ${PLUGIN_SOURCES}
    ${UI_SOURCES}
)

# Add test-specific compile definitions
target_compile_definitions(QuackerVSTTests PRIVATE
    QUACKER_TEST_BUILD=1
    JUCE_UNIT_TESTS=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    
    # Plugin identification
    JucePlugin_Name="TremoloViola"
    JucePlugin_Desc="Tremolo Viola"
    JucePlugin_Manufacturer="Acedia Audio"
    JucePlugin_ManufacturerWebsite="https://acediaaudio.com"
    JucePlugin_ManufacturerEmail="deividshvostovsbusiness@gmail.com"
    JucePlugin_ManufacturerCode=0x41636564
    JucePlugin_PluginCode=0x54726D56
    JucePlugin_IsSynth=0
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_EditorRequiresKeyboardFocus=0
    JucePlugin_Version=1.0.0
    JucePlugin_VersionCode=0x10000
    JucePlugin_VersionString="1.0.0"
    JucePlugin_VSTUniqueID=JucePlugin_PluginCode
    JucePlugin_VSTCategory=kPlugCategEffect
    JucePlugin_Vst3Category="Fx"
    JucePlugin_AUMainType=kAudioUnitType_Effect
    JucePlugin_AUSubType=JucePlugin_PluginCode
    JucePlugin_AUExportPrefix=TremoloViolaAU
    JucePlugin_AUExportPrefixQuoted="TremoloViolaAU"
    JucePlugin_AUManufacturerCode=JucePlugin_ManufacturerCode
    JucePlugin_CFBundleIdentifier=com.acediaaudio.TremoloViola
    JucePlugin_AAXIdentifier=com.acediaaudio.TremoloViola
    JucePlugin_AAXManufacturerCode=JucePlugin_ManufacturerCode
    JucePlugin_AAXProductId=JucePlugin_PluginCode
    JucePlugin_AAXCategory=0
    JucePlugin_AAXDisableBypass=0
    JucePlugin_AAXDisableMultiMono=0
    JucePlugin_IAAType=0x61757278
    JucePlugin_IAASubType=JucePlugin_PluginCode
    JucePlugin_IAAName="Acedia Audio: TremoloViola"
    JucePlugin_VSTNumMidiInputs=0
    JucePlugin_VSTNumMidiOutputs=0
    JucePlugin_MaxNumInputChannels=2
    JucePlugin_MaxNumOutputChannels=2
    JucePlugin_PreferredChannelConfigurations={1,1},{2,2}
)

# Link JUCE modules
target_link_libraries(QuackerVSTTests
    PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Include directories for plugin headers
target_include_directories(QuackerVSTTests PRIVATE
    QuackerVST/Source
    Tests
)

# Handle BinaryData if it exists (for fonts, etc.)
set(BINARY_DATA_FOUND FALSE)
set(BINARY_DATA_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/JuceLibraryCode/BinaryData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/Builds/MacOSX/BinaryData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/Builds/VisualStudio2022/BinaryData.cpp"
)

foreach(BINARY_DATA_PATH ${BINARY_DATA_PATHS})
    if(EXISTS ${BINARY_DATA_PATH})
        message(STATUS "Found BinaryData at: ${BINARY_DATA_PATH}")
        target_sources(QuackerVSTTests PRIVATE ${BINARY_DATA_PATH})
        get_filename_component(BINARY_DATA_DIR ${BINARY_DATA_PATH} DIRECTORY)
        target_include_directories(QuackerVSTTests PRIVATE ${BINARY_DATA_DIR})
        set(BINARY_DATA_FOUND TRUE)
        break()
    endif()
endforeach()

if(NOT BINARY_DATA_FOUND)
    message(WARNING "BinaryData.cpp not found - fonts may not work in tests")
    target_compile_definitions(QuackerVSTTests PRIVATE NO_BINARY_DATA=1)
endif()

# Platform-specific settings
if(MSVC)
    target_compile_options(QuackerVSTTests PRIVATE /W4 /bigobj)
    # Disable some warnings that are common in JUCE projects
    target_compile_options(QuackerVSTTests PRIVATE /wd4996 /wd4100 /wd4127)
else()
    target_compile_options(QuackerVSTTests PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

# Set output directory
set_target_properties(QuackerVSTTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
)