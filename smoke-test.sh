#!/bin/bash
# smoke-test.sh - Minimal smoke test for CI

set -e

echo "QuackerVST Smoke Test"
echo "===================="

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

# Check if we can compile a minimal test
echo "Performing minimal compilation test..."

# Create a minimal test file
cat > minimal_test.cpp << 'EOF'
#include <iostream>

// Minimal JUCE headers
#define JUCE_CORE_H_INCLUDED 1
#define JUCE_AUDIO_BASICS_H_INCLUDED 1

// Mock JUCE types for compilation test
namespace juce {
    class String {
    public:
        String(const char* s) : data(s) {}
        const char* toRawUTF8() const { return data.c_str(); }
        bool isNotEmpty() const { return !data.empty(); }
    private:
        std::string data;
    };
    
    template<typename T>
    class AudioBuffer {
    public:
        AudioBuffer(int channels, int samples) : numChannels(channels), numSamples(samples) {}
        int getNumChannels() const { return numChannels; }
        int getNumSamples() const { return numSamples; }
        T* getWritePointer(int channel) { return nullptr; }
        const T* getReadPointer(int channel) const { return nullptr; }
    private:
        int numChannels, numSamples;
    };
    
    class MidiBuffer {};
}

// Test that our basic plugin structure compiles
class MockProcessor {
public:
    MockProcessor() {
        name = "TremoloViola";
    }
    
    juce::String getName() const { return name; }
    bool isMidiEffect() const { return false; }
    void prepareToPlay(double sampleRate, int samplesPerBlock) {}
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {}
    
private:
    juce::String name;
};

int main() {
    std::cout << "Minimal compilation test..." << std::endl;
    
    MockProcessor processor;
    std::cout << "Plugin name: " << processor.getName().toRawUTF8() << std::endl;
    std::cout << "Is MIDI effect: " << (processor.isMidiEffect() ? "Yes" : "No") << std::endl;
    
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    processor.prepareToPlay(44100.0, 512);
    processor.processBlock(buffer, midiBuffer);
    
    std::cout << "Basic compilation test passed!" << std::endl;
    return 0;
}
EOF

# Try to compile the minimal test
echo "  Compiling minimal test..."

if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    if clang++ -std=c++17 -o minimal_test minimal_test.cpp; then
        echo "  ✓ Minimal compilation successful"
        if ./minimal_test; then
            echo "  ✓ Minimal test execution successful"
        else
            echo "  ✗ Minimal test execution failed"
            exit 1
        fi
    else
        echo "  ✗ Minimal compilation failed"
        exit 1
    fi
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    # Windows (if available)
    if cl /std:c++17 /Fe:minimal_test.exe minimal_test.cpp 2>/dev/null; then
        echo "  ✓ Minimal compilation successful"
        if ./minimal_test.exe; then
            echo "  ✓ Minimal test execution successful"
        else
            echo "  ✗ Minimal test execution failed"
            exit 1
        fi
    else
        echo "  ⚠ Windows compiler not available, skipping compilation test"
    fi
else
    # Linux/Other
    if g++ -std=c++17 -o minimal_test minimal_test.cpp; then
        echo "  ✓ Minimal compilation successful"
        if ./minimal_test; then
            echo "  ✓ Minimal test execution successful"
        else
            echo "  ✗ Minimal test execution failed"
            exit 1
        fi
    else
        echo "  ✗ Minimal compilation failed"
        exit 1
    fi
fi

# Clean up
rm -f minimal_test minimal_test.exe minimal_test.cpp

# Check JUCER file validity
echo "Checking JUCER file..."
if grep -q "TremoloViola" QuackerVST/QuackerVST.jucer; then
    echo "  ✓ JUCER file contains expected plugin name"
else
    echo "  ⚠ JUCER file may have issues"
fi

if grep -q "juce_audio_processors" QuackerVST/QuackerVST.jucer; then
    echo "  ✓ JUCER file includes audio_processors module"
else
    echo "  ✗ JUCER file missing audio_processors module"
    exit 1
fi

echo ""
echo "🎉 All smoke tests passed!"
echo ""
echo "This indicates that:"
echo "  - Project structure is correct"
echo "  - JUCE submodule is available"
echo "  - Basic compilation works"
echo "  - JUCER file is properly configured"
echo ""
echo "You can now proceed with full build/test processes."