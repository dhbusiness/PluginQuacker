#!/bin/bash
# clean-repo.sh - Clean up repository for CI

echo "Cleaning up repository for CI build..."

# Remove build artifacts that shouldn't be in version control
echo "Removing build artifacts..."
rm -rf build-tests/
rm -rf build/
rm -rf CMakeFiles/
rm -f CMakeCache.txt
rm -f cmake_install.cmake
rm -f Makefile
rm -f test_results.xml
rm -f test_diagnostics.log
rm -f *_test_log.txt

# Remove platform-specific files
echo "Removing platform-specific files..."
find . -name ".DS_Store" -delete
find . -name "Thumbs.db" -delete
find . -name "*.user" -delete

# Remove JUCE build artifacts (but keep the source)
echo "Cleaning JUCE build artifacts..."
find QuackerVST/JUCE -name "build" -type d -exec rm -rf {} + 2>/dev/null || true
find QuackerVST/JUCE -name "Builds" -type d -exec rm -rf {} + 2>/dev/null || true

# Remove plugin build artifacts (but keep source)
echo "Cleaning plugin build artifacts..."
find QuackerVST/Builds -name "build" -type d -exec rm -rf {} + 2>/dev/null || true
find QuackerVST/Builds -name "*.xcworkspace" -exec rm -rf {} + 2>/dev/null || true
find QuackerVST/Builds -name "*.xcodeproj/project.xcworkspace" -exec rm -rf {} + 2>/dev/null || true
find QuackerVST/Builds -name "*.xcodeproj/xcuserdata" -exec rm -rf {} + 2>/dev/null || true

# Clean Visual Studio artifacts
find . -name "*.vcxproj.user" -delete
find . -name "*.vcxproj.filters" -delete
find . -name "*.sln" -not -path "*/JUCE/*" -delete 2>/dev/null || true
find . -name "x64" -type d -not -path "*/JUCE/*" -exec rm -rf {} + 2>/dev/null || true
find . -name "Debug" -type d -not -path "*/JUCE/*" -exec rm -rf {} + 2>/dev/null || true
find . -name "Release" -type d -not -path "*/JUCE/*" -exec rm -rf {} + 2>/dev/null || true

echo "Repository cleaned successfully!"
echo ""
echo "Next steps:"
echo "1. Add all changes to git: git add ."
echo "2. Commit the changes: git commit -m 'Clean up build artifacts and fix CI'"
echo "3. Push to trigger CI: git push"
echo ""
echo "Make sure you have the following files:"
echo "- .gitignore (to prevent future build artifacts)"
echo "- CMakeLists.txt (updated for better CI compatibility)"
echo "- Tests/TestRunner.cpp (fixed header includes)"
echo "- Tests/UIStubs.cpp (for headless testing)"