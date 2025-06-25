#!/bin/bash
# build-tests.sh - Build and run tests on macOS/Linux

set -e

echo "Building QuackerVST Tests..."

# Create build directory
mkdir -p build-tests
cd build-tests

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "Build complete!"
echo "Running tests..."

# Run tests
./QuackerVSTTests --verbose

echo "Tests complete!"