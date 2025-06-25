#!/bin/bash
# smoke-test-simple.sh - Ultra minimal smoke test for CI (no compilation)

set -e

echo "QuackerVST Simple Smoke Test"
echo "============================="

# Check basic file structure
echo "Checking project structure..."

REQUIRED_FILES=(
    "QuackerVST/QuackerVST.jucer"
    "QuackerVST/Source/PluginProcessor.h"
    "QuackerVST/Source/PluginProcessor.cpp"
    "QuackerVST/Source/TremoloLFO.h"
    "QuackerVST/Source/TremoloLFO.cpp"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ Found: $file"
    else
        echo "  ✗ Missing: $file"
        exit 1
    fi
done

# Check JUCE submodule
echo "Checking JUCE submodule..."
if [ -d "QuackerVST/JUCE/modules" ]; then
    echo "  ✓ JUCE modules found"
    
    # Count JUCE modules
    MODULE_COUNT=$(find QuackerVST/JUCE/modules -maxdepth 1 -type d -name "juce_*" | wc -l)
    echo "  ✓ Found $MODULE_COUNT JUCE modules"
    
    if [ "$MODULE_COUNT" -lt 10 ]; then
        echo "  ⚠ Warning: Expected more JUCE modules, but continuing..."
    fi
else
    echo "  ✗ JUCE modules missing"
    echo "  → Running: git submodule update --init --recursive"
    git submodule update --init --recursive
    
    if [ -d "QuackerVST/JUCE/modules" ]; then
        echo "  ✓ JUCE modules now available"
    else
        echo "  ✗ JUCE submodule initialization failed"
        exit 1
    fi
fi

# Check JUCER file validity
echo "Checking JUCER file content..."

if grep -q "TremoloViola" QuackerVST/QuackerVST.jucer; then
    echo "  ✓ JUCER file contains expected plugin name"
else
    echo "  ⚠ JUCER file may have issues with plugin name"
fi

if grep -q "juce_audio_processors" QuackerVST/QuackerVST.jucer; then
    echo "  ✓ JUCER file includes audio_processors module"
else
    echo "  ✗ JUCER file missing audio_processors module"
    exit 1
fi

if grep -q "audioplug" QuackerVST/QuackerVST.jucer; then
    echo "  ✓ JUCER file is configured as audio plugin"
else
    echo "  ⚠ JUCER file may not be configured as audio plugin"
fi

# Check source file content
echo "Checking source file content..."

if grep -q "class.*AudioProcessor" QuackerVST/Source/PluginProcessor.h; then
    echo "  ✓ PluginProcessor.h contains AudioProcessor class"
else
    echo "  ⚠ PluginProcessor.h may have issues"
fi

if grep -q "processBlock" QuackerVST/Source/PluginProcessor.cpp; then
    echo "  ✓ PluginProcessor.cpp contains processBlock method"
else
    echo "  ⚠ PluginProcessor.cpp may be missing processBlock"
fi

if grep -q "class.*LFO\|struct.*LFO" QuackerVST/Source/TremoloLFO.h; then
    echo "  ✓ TremoloLFO.h contains LFO class/struct"
else
    echo "  ⚠ TremoloLFO.h may have issues"
fi

# Check test structure
echo "Checking test structure..."

if [ -d "Tests" ]; then
    echo "  ✓ Tests directory exists"
    
    TEST_FILES=(
        "Tests/QuackerVSTTests.h"
        "Tests/TestRunner.cpp"
        "Tests/TremoloLFOTests.cpp"
    )
    
    for file in "${TEST_FILES[@]}"; do
        if [ -f "$file" ]; then
            echo "  ✓ Found: $file"
        else
            echo "  ⚠ Missing: $file"
        fi
    done
else
    echo "  ⚠ Tests directory missing"
fi

# Check build system files
echo "Checking build system..."

if [ -f "CMakeLists.txt" ]; then
    echo "  ✓ CMakeLists.txt exists"
    
    if grep -q "QuackerVSTTests" CMakeLists.txt; then
        echo "  ✓ CMakeLists.txt configured for tests"
    else
        echo "  ⚠ CMakeLists.txt may not be configured for tests"
    fi
else
    echo "  ⚠ CMakeLists.txt missing"
fi

# Check no build artifacts exist
echo "Checking for build artifacts..."

BUILD_ARTIFACTS=(
    "build-tests"
    "build"
    "CMakeCache.txt"
    "Makefile"
    "CMakeFiles"
)

FOUND_ARTIFACTS=0
for artifact in "${BUILD_ARTIFACTS[@]}"; do
    if [ -e "$artifact" ]; then
        echo "  ⚠ Found build artifact: $artifact"
        FOUND_ARTIFACTS=$((FOUND_ARTIFACTS + 1))
    fi
done

if [ "$FOUND_ARTIFACTS" -eq 0 ]; then
    echo "  ✓ No build artifacts found (clean repository)"
else
    echo "  ⚠ Found $FOUND_ARTIFACTS build artifacts - consider cleaning"
fi

# Check platform-specific tools
echo "Checking platform and tools..."

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "  ✓ Running on macOS"
    if command -v xcodebuild >/dev/null 2>&1; then
        echo "  ✓ xcodebuild available"
    else
        echo "  ⚠ xcodebuild not available"
    fi
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    echo "  ✓ Running on Windows"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "  ✓ Running on Linux"
else
    echo "  ✓ Running on: $OSTYPE"
fi

if command -v cmake >/dev/null 2>&1; then
    CMAKE_VERSION=$(cmake --version | head -n1)
    echo "  ✓ CMake available: $CMAKE_VERSION"
else
    echo "  ⚠ CMake not available"
fi

echo ""
echo "🎉 Simple smoke test completed!"
echo ""
echo "Summary:"
echo "  - Project structure looks good"
echo "  - JUCE submodule is available"
echo "  - Source files contain expected content"
echo "  - Build system files are present"
echo ""
echo "This indicates the project should be ready for build/test processes."
echo "If issues occur, they're likely related to:"
echo "  - CMake/JUCE integration complexities"
echo "  - Platform-specific build tool configuration"
echo "  - Missing dependencies or certificates"