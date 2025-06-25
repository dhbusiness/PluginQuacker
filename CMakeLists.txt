cmake_minimum_required(VERSION 3.15)
project(QuackerVSTTests VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/QuackerVST/JUCE juce_build)

# Create a JUCE console app instead of plain executable
juce_add_console_app(QuackerVSTTests
    PRODUCT_NAME "QuackerVST Tests"
    COMPANY_NAME "Acedia Audio"
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE)

# Add source files
target_sources(QuackerVSTTests PRIVATE
    # Test files
    Tests/TestRunner.cpp
    Tests/TremoloLFOTests.cpp
    Tests/PresetManagerTests.cpp
    Tests/PluginProcessorTests.cpp
    
    # Plugin source files (core functionality)
    QuackerVST/Source/PluginProcessor.cpp
    QuackerVST/Source/TremoloLFO.cpp
    QuackerVST/Source/PresetManager.cpp
    QuackerVST/Source/WaveshapeLFO.cpp
    QuackerVST/Source/PerlinNoise.cpp
    QuackerVST/Source/Presets.cpp
    
    # UI components needed for compilation (but won't be used in tests)
    QuackerVST/Source/HierarchicalPresetMenu.cpp
    QuackerVST/Source/Fonts/FontManager.cpp
    
    # UI stub for headless testing
    Tests/UIStubs.cpp
)

# Add test-specific compile definitions
target_compile_definitions(QuackerVSTTests PRIVATE
    QUACKER_TEST_BUILD=1
)

# Preprocessor definitions
target_compile_definitions(QuackerVSTTests
    PUBLIC
    JUCE_UNIT_TESTS=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JucePlugin_Name="TremoloViola"
    JucePlugin_Desc="Tremolo Viola"
    JucePlugin_Manufacturer="Acedia Audio"
    JucePlugin_ManufacturerWebsite="https://acediaaudio.com"
    JucePlugin_ManufacturerEmail="deividshvostovsbusiness@gmail.com"
    JucePlugin_ManufacturerCode=0x41636564  # 'Aced'
    JucePlugin_PluginCode=0x54726D56         # 'TrmV'
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

# Include BinaryData if it exists
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
        break()
    endif()
endforeach()

# If no BinaryData found, add a compile definition
if(NOT EXISTS ${BINARY_DATA_PATH})
    message(WARNING "BinaryData.cpp not found - fonts may not work")
    target_compile_definitions(QuackerVSTTests PRIVATE NO_BINARY_DATA=1)
endif()

# Platform-specific settings
if(MSVC)
    target_compile_options(QuackerVSTTests PRIVATE /W4 /bigobj)
else()
    target_compile_options(QuackerVSTTests PRIVATE -Wall -Wextra)
endif()