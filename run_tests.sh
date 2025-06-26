#!/bin/bash

# TremoloViola Plugin Test Runner
# This script builds and runs the test suite locally

set -e  # Exit on any error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TEST_DIR="$PROJECT_ROOT/Tests"
PLUGIN_DIR="$PROJECT_ROOT/QuackerVST"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
TEST_TYPE="all"
VERBOSE=false
BUILD_ONLY=false
CLEAN_BUILD=false

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show help
show_help() {
    cat << EOF
TremoloViola Plugin Test Runner

Usage: $0 [OPTIONS]

Options:
    --smoke             Run smoke tests only
    --unit              Run unit tests only
    --all               Run all tests (default)
    --verbose, -v       Enable verbose output
    --build-only        Build tests but don't run them
    --clean             Clean build directories before building
    --help, -h          Show this help message

Examples:
    $0                  # Run all tests
    $0 --smoke          # Run smoke tests only
    $0 --unit --verbose # Run unit tests with verbose output
    $0 --clean --all    # Clean build and run all tests
    $0 --build-only     # Just build the test executable

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --smoke)
            TEST_TYPE="smoke"
            shift
            ;;
        --unit)
            TEST_TYPE="unit"
            shift
            ;;
        --all)
            TEST_TYPE="all"
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --build-only)
            BUILD_ONLY=true
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Function to detect OS and set build paths
detect_platform() {
    case "$(uname -s)" in
        Darwin)
            PLATFORM="macos"
            BUILD_DIR="$TEST_DIR/Builds/MacOSX"
            PROJUCER_BUILD_DIR="$PLUGIN_DIR/JUCE/extras/Projucer/Builds/MacOSX"
            PROJUCER_EXECUTABLE="$PROJUCER_BUILD_DIR/build/Release/Projucer.app/Contents/MacOS/Projucer"
            TEST_EXECUTABLE="$BUILD_DIR/build/Release/TremoloViola Tests"
            ;;
        Linux)
            PLATFORM="linux"
            BUILD_DIR="$TEST_DIR/Builds/LinuxMakefile"
            PROJUCER_BUILD_DIR="$PLUGIN_DIR/JUCE/extras/Projucer/Builds/LinuxMakefile"
            PROJUCER_EXECUTABLE="$PROJUCER_BUILD_DIR/build/Projucer"
            TEST_EXECUTABLE="$BUILD_DIR/build/TremoloViola Tests"
            ;;
        CYGWIN*|MINGW32*|MSYS*|MINGW*)
            print_error "Please use run_tests.ps1 on Windows"
            exit 1
            ;;
        *)
            print_error "Unsupported platform: $(uname -s)"
            exit 1
            ;;
    esac
    
    print_info "Detected platform: $PLATFORM"
}

# Function to check dependencies
check_dependencies() {
    print_info "Checking dependencies..."
    
    # Check for git submodules
    if [ ! -f "$PLUGIN_DIR/JUCE/CMakeLists.txt" ]; then
        print_info "JUCE submodule not found, initializing..."
        cd "$PROJECT_ROOT"
        git submodule update --init --recursive
    fi
    
    case $PLATFORM in
        macos)
            if ! command -v xcodebuild &> /dev/null; then
                print_error "xcodebuild not found. Please install Xcode."
                exit 1
            fi
            ;;
        linux)
            if ! command -v make &> /dev/null; then
                print_error "make not found. Please install build-essential."
                exit 1
            fi
            if ! command -v g++ &> /dev/null; then
                print_error "g++ not found. Please install g++."
                exit 1
            fi
            # Check for JUCE dependencies on Linux
            print_info "Checking Linux dependencies..."
            MISSING_DEPS=()
            
            if ! pkg-config --exists alsa; then
                MISSING_DEPS+=("libasound2-dev")
            fi
            if ! pkg-config --exists freetype2; then
                MISSING_DEPS+=("libfreetype6-dev")
            fi
            if ! pkg-config --exists x11; then
                MISSING_DEPS+=("libx11-dev")
            fi
            
            if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
                print_warning "Missing dependencies: ${MISSING_DEPS[*]}"
                print_info "Install with: sudo apt-get install ${MISSING_DEPS[*]}"
            fi
            ;;
    esac
}

# Function to clean build directories
clean_builds() {
    if [ "$CLEAN_BUILD" = true ]; then
        print_info "Cleaning build directories..."
        
        if [ -d "$BUILD_DIR" ]; then
            rm -rf "$BUILD_DIR/build"
            print_info "Cleaned test build directory"
        fi
        
        if [ -d "$PROJUCER_BUILD_DIR" ]; then
            rm -rf "$PROJUCER_BUILD_DIR/build"
            print_info "Cleaned Projucer build directory"
        fi
    fi
}

# Function to build Projucer
build_projucer() {
    print_info "Building Projucer..."
    
    if [ -f "$PROJUCER_EXECUTABLE" ] && [ "$CLEAN_BUILD" != true ]; then
        print_info "Projucer already built, skipping..."
        return 0
    fi
    
    cd "$PROJUCER_BUILD_DIR"
    
    case $PLATFORM in
        macos)
            xcodebuild -configuration Release -quiet
            ;;
        linux)
            make CONFIG=Release -j$(nproc)
            ;;
    esac
    
    if [ ! -f "$PROJUCER_EXECUTABLE" ]; then
        print_error "Failed to build Projucer"
        exit 1
    fi
    
    print_success "Projucer built successfully"
}

# Function to generate test project
generate_test_project() {
    print_info "Generating test project..."
    
    cd "$PROJECT_ROOT"
    "$PROJUCER_EXECUTABLE" --resave "$TEST_DIR/TremoloViola_Tests.jucer"
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Failed to generate test project"
        exit 1
    fi
    
    print_success "Test project generated"
}

# Function to build tests
build_tests() {
    print_info "Building test application..."
    
    cd "$BUILD_DIR"
    
    case $PLATFORM in
        macos)
            xcodebuild -configuration Release -quiet -target "TremoloViola Tests"
            ;;
        linux)
            make CONFIG=Release -j$(nproc)
            ;;
    esac
    
    if [ ! -f "$TEST_EXECUTABLE" ]; then
        print_error "Failed to build test application"
        exit 1
    fi
    
    print_success "Test application built successfully"
}

# Function to create test source files
create_test_sources() {
    print_info "Creating test source files..."
    
    # Create the Tests/Source directory if it doesn't exist
    mkdir -p "$TEST_DIR/Source"
    
    # Check if test files exist, if not, warn the user
    if [ ! -f "$TEST_DIR/Source/Main.cpp" ]; then
        print_warning "Test source files not found in $TEST_DIR/Source/"
        print_warning "Please create the test files using the provided artifacts:"
        print_warning "- Main.cpp"
        print_warning "- SmokeTests.h and SmokeTests.cpp"
        print_warning "- TremoloLFOTests.h and TremoloLFOTests.cpp"
        print_warning "- PresetManagerTests.h and PresetManagerTests.cpp"
        print_warning "- PluginTests.h and PluginTests.cpp"
        return 1
    fi
    
    return 0
}

# Function to run tests
run_tests() {
    if [ "$BUILD_ONLY" = true ]; then
        print_success "Build completed successfully (build-only mode)"
        return 0
    fi
    
    print_info "Running tests..."
    
    # Prepare test arguments
    VERBOSE_FLAG=""
    if [ "$VERBOSE" = true ]; then
        VERBOSE_FLAG="--verbose"
    fi
    
    # Run the specified test type
    cd "$(dirname "$TEST_EXECUTABLE")"
    case $TEST_TYPE in
        smoke)
            print_info "Running smoke tests..."
            "$TEST_EXECUTABLE" --smoke $VERBOSE_FLAG
            ;;
        unit)
            print_info "Running unit tests..."
            "$TEST_EXECUTABLE" --unit $VERBOSE_FLAG
            ;;
        all)
            print_info "Running all tests..."
            "$TEST_EXECUTABLE" --all $VERBOSE_FLAG
            ;;
    esac
    
    local exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        print_success "All tests passed! 🎉"
    else
        print_error "Tests failed with exit code $exit_code"
        exit $exit_code
    fi
}

# Main execution
main() {
    print_info "TremoloViola Plugin Test Runner"
    print_info "==============================="
    
    detect_platform
    check_dependencies
    clean_builds
    
    if ! create_test_sources; then
        print_error "Test source files are missing. Please create them first."
        exit 1
    fi
    
    build_projucer
    generate_test_project
    build_tests
    run_tests
    
    print_success "Test run completed successfully!"
}

# Run main function
main "$@"