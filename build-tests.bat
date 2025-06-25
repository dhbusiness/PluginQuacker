@echo off
REM build-tests.bat - Build and run tests on Windows

echo Building QuackerVST Tests...

REM Create build directory
if not exist build-tests mkdir build-tests
cd build-tests

REM Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

REM Build
cmake --build . --config Debug

echo Build complete!
echo Running tests...

REM Run tests
Debug\QuackerVSTTests.exe --verbose

echo Tests complete!
pause