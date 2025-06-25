#!/bin/bash
# build-tests-simple.sh - Simple build script that bypasses CMake/JUCE issues

set -e

echo "Simple QuackerVST Test Builder"
echo "=============================="

# Check if JUCE is available
if [ ! -d "QuackerVST/JUCE/modules" ]; then
    echo "ERROR: JUCE modules not found. Run 'git submodule update --init --recursive'"
    exit 1
fi

# Create build directory
BUILD_DIR="build-simple"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Detect platform
PLATFORM=""
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
else
    PLATFORM="linux"
fi

echo "Building for platform: $PLATFORM"

# Set compiler and flags based on platform
if [ "$PLATFORM" = "macos" ]; then
    CXX="clang++"
    CXXFLAGS="-std=c++17 -O2 -DJUCE_MAC=1"
    FRAMEWORKS="-framework Foundation -framework Cocoa -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreMIDI -framework Accelerate -framework QuartzCore -framework IOKit -framework Security"
    JUCE_COMPILE_FLAGS="-x objective-c++"
    OBJ_EXT=".o"
    EXE_NAME="QuackerVSTTests"
elif [ "$PLATFORM" = "windows" ]; then
    CXX="cl"
    CXXFLAGS="/std:c++17 /O2 /DJUCE_WINDOWS=1 /D_CRT_SECURE_NO_WARNINGS=1"
    FRAMEWORKS="winmm.lib ole32.lib uuid.lib wsock32.lib wininet.lib shlwapi.lib version.lib"
    JUCE_COMPILE_FLAGS=""
    OBJ_EXT=".obj"
    EXE_NAME="QuackerVSTTests.exe"
else
    echo "Linux not fully supported in this simple script"
    exit 1
fi

# Common definitions
COMMON_DEFS="-DJUCE_STANDALONE_APPLICATION=1 -DJUCE_USE_CURL=0 -DJUCE_WEB_BROWSER=0 -DJUCE_UNIT_TESTS=1 -DQUACKER_TEST_BUILD=1"

# JUCE module definitions
JUCE_DEFS="-DJUCE_MODULE_AVAILABLE_juce_core=1 -DJUCE_MODULE_AVAILABLE_juce_audio_basics=1 -DJUCE_MODULE_AVAILABLE_juce_audio_devices=1 -DJUCE_MODULE_AVAILABLE_juce_audio_formats=1 -DJUCE_MODULE_AVAILABLE_juce_audio_processors=1 -DJUCE_MODULE_AVAILABLE_juce_audio_utils=1 -DJUCE_MODULE_AVAILABLE_juce_data_structures=1 -DJUCE_MODULE_AVAILABLE_juce_dsp=1 -DJUCE_MODULE_AVAILABLE_juce_events=1 -DJUCE_MODULE_AVAILABLE_juce_graphics=1 -DJUCE_MODULE_AVAILABLE_juce_gui_basics=1 -DJUCE_MODULE_AVAILABLE_juce_gui_extra=1"

# Plugin definitions
PLUGIN_DEFS='-DJucePlugin_Name="TremoloViola" -DJucePlugin_Manufacturer="Acedia Audio" -DJucePlugin_IsSynth=0 -DJucePlugin_WantsMidiInput=0 -DJucePlugin_ProducesMidiOutput=0'

# Include paths
INCLUDES="-IQuackerVST/JUCE/modules -IQuackerVST/Source -ITests"

# All definitions
ALL_DEFS="$COMMON_DEFS $JUCE_DEFS $PLUGIN_DEFS"

echo "Compiling JUCE modules..."

# Function to compile a source file
compile_file() {
    local src_file="$1"
    local obj_file="$BUILD_DIR/$(basename "$src_file" .cpp)$OBJ_EXT"
    local extra_flags="$2"
    
    echo "  Compiling $src_file..."
    
    if [ "$PLATFORM" = "macos" ]; then
        $CXX $CXXFLAGS $extra_flags $ALL_DEFS $INCLUDES -c "$src_file" -o "$obj_file"
    elif [ "$PLATFORM" = "windows" ]; then
        $CXX $CXXFLAGS $ALL_DEFS $INCLUDES /c "$src_file" /Fo:"$obj_file"
    fi
    
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to compile $src_file"
        exit 1
    fi
}

# Compile JUCE modules
JUCE_MODULES=(
    "QuackerVST/JUCE/modules/juce_core/juce_core.cpp"
    "QuackerVST/JUCE/modules/juce_audio_basics/juce_audio_basics.cpp"
    "QuackerVST/JUCE/modules/juce_audio_devices/juce_audio_devices.cpp"
    "QuackerVST/JUCE/modules/juce_audio_formats/juce_audio_formats.cpp"
    "QuackerVST/JUCE/modules/juce_audio_processors/juce_audio_processors.cpp"
    "QuackerVST/JUCE/modules/juce_audio_utils/juce_audio_utils.cpp"
    "QuackerVST/JUCE/modules/juce_data_structures/juce_data_structures.cpp"
    "QuackerVST/JUCE/modules/juce_dsp/juce_dsp.cpp"
    "QuackerVST/JUCE/modules/juce_events/juce_events.cpp"
    "QuackerVST/JUCE/modules/juce_graphics/juce_graphics.cpp"
    "QuackerVST/JUCE/modules/juce_gui_basics/juce_gui_basics.cpp"
    "QuackerVST/JUCE/modules/juce_gui_extra/juce_gui_extra.cpp"
)

for module in "${JUCE_MODULES[@]}"; do
    if [ -f "$module" ]; then
        compile_file "$module" "$JUCE_COMPILE_FLAGS"
    else
        echo "WARNING: JUCE module not found: $module"
    fi
done

echo "Compiling plugin sources..."

# Compile plugin sources
PLUGIN_SOURCES=(
    "QuackerVST/Source/PluginProcessor.cpp"
    "QuackerVST/Source/TremoloLFO.cpp"
    "QuackerVST/Source/PresetManager.cpp"
    "QuackerVST/Source/WaveshapeLFO.cpp"
    "QuackerVST/Source/PerlinNoise.cpp"
    "QuackerVST/Source/Presets.cpp"
)

for src in "${PLUGIN_SOURCES[@]}"; do
    if [ -f "$src" ]; then
        compile_file "$src"
    else
        echo "WARNING: Plugin source not found: $src"
    fi
done

# Compile optional UI sources
if [ -f "QuackerVST/Source/HierarchicalPresetMenu.cpp" ]; then
    compile_file "QuackerVST/Source/HierarchicalPresetMenu.cpp"
fi

if [ -f "QuackerVST/Source/Fonts/FontManager.cpp" ]; then
    compile_file "QuackerVST/Source/Fonts/FontManager.cpp"
fi

echo "Compiling test sources..."

# Compile test sources
TEST_SOURCES=(
    "Tests/TestRunner.cpp"
    "Tests/TremoloLFOTests.cpp"
    "Tests/PresetManagerTests.cpp"
    "Tests/PluginProcessorTests.cpp"
)

for src in "${TEST_SOURCES[@]}"; do
    if [ -f "$src" ]; then
        compile_file "$src"
    else
        echo "ERROR: Test source not found: $src"
        exit 1
    fi
done

echo "Linking executable..."

# Link everything together
if [ "$PLATFORM" = "macos" ]; then
    $CXX $CXXFLAGS -o "$BUILD_DIR/$EXE_NAME" "$BUILD_DIR"/*$OBJ_EXT $FRAMEWORKS
elif [ "$PLATFORM" = "windows" ]; then
    $CXX /Fe:"$BUILD_DIR/$EXE_NAME" "$BUILD_DIR"/*$OBJ_EXT $FRAMEWORKS
fi

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "Test executable: $BUILD_DIR/$EXE_NAME"
    echo ""
    echo "To run tests:"
    echo "  cd $BUILD_DIR && ./$EXE_NAME --verbose"
else
    echo "❌ Link failed!"
    exit 1
fi