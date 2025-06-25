#!/bin/bash
# cleanup-tests.sh - Remove redundant test files and clean build artifacts

echo "Cleaning up redundant test files..."

# Remove redundant test files
rm -f Tests/MinimalTest.cpp
rm -f Tests/SimpleTestRunner.cpp
rm -f Tests/SimplifiedTests.h
rm -f Tests/PluginProcessorTests.cpp  # Keep PluginProcessorTests_Fixed.cpp
rm -f Tests/JuceHeader.h
rm -f Tests/UIStubs.cpp

echo "Removed redundant test files"

# Clean build artifacts
echo "Cleaning build artifacts..."
rm -rf build-tests/
rm -rf build-simple/
rm -rf build/
rm -f CMakeCache.txt
rm -f cmake_install.cmake
rm -f Makefile
rm -f test_results.xml
rm -f test_diagnostics.log
rm -f *_test_log.txt
rm -f minimal_test
rm -f minimal_test.exe
rm -f minimal_test.cpp

# Clean Xcode build artifacts in plugin directory
echo "Cleaning Xcode build artifacts..."
rm -rf QuackerVST/Builds/MacOSX/*.xcodeproj
rm -rf QuackerVST/Builds/MacOSX/build/

# Clean Visual Studio build artifacts
echo "Cleaning Visual Studio build artifacts..."
rm -f QuackerVST/Builds/VisualStudio2022/*.sln
rm -f QuackerVST/Builds/VisualStudio2022/*.vcxproj
rm -f QuackerVST/Builds/VisualStudio2022/*.vcxproj.filters
rm -rf QuackerVST/Builds/VisualStudio2022/x64/
rm -rf QuackerVST/Builds/VisualStudio2022/Debug/
rm -rf QuackerVST/Builds/VisualStudio2022/Release/

echo "Clean-up complete!"
echo ""
echo "Remaining test files:"
ls -la Tests/
echo ""
echo "Next steps:"
echo "1. Review and update the GitHub Actions workflow (.github/workflows/build-plugin.yml)"
echo "2. Commit these changes"
echo "3. Push to trigger the CI pipeline"