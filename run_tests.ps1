# TremoloViola Plugin Test Runner for Windows
# This script builds and runs the test suite locally using PowerShell

param(
    [switch]$Smoke,
    [switch]$Unit,
    [switch]$All,
    [switch]$Verbose,
    [switch]$BuildOnly,
    [switch]$Clean,
    [switch]$Help
)

# Configuration
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$TestDir = Join-Path $ProjectRoot "Tests"
$PluginDir = Join-Path $ProjectRoot "QuackerVST"

# Build paths for Windows
$BuildDir = Join-Path $TestDir "Builds\VisualStudio2022"
$ProjectorBuildDir = Join-Path $PluginDir "JUCE\extras\Projucer\Builds\VisualStudio2022"
$ProjectorExecutable = Join-Path $ProjectorBuildDir "x64\Release\App\Projucer.exe"
$TestExecutable = Join-Path $BuildDir "x64\Release\App\TremoloViola Tests.exe"

# Default values
$TestType = "all"
if ($Smoke) { $TestType = "smoke" }
if ($Unit) { $TestType = "unit" }
if ($All) { $TestType = "all" }

# Function to print colored output
function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Blue
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# Function to show help
function Show-Help {
    @"
TremoloViola Plugin Test Runner for Windows

Usage: .\run_tests.ps1 [OPTIONS]

Options:
    -Smoke              Run smoke tests only
    -Unit               Run unit tests only
    -All                Run all tests (default)
    -Verbose            Enable verbose output
    -BuildOnly          Build tests but don't run them
    -Clean              Clean build directories before building
    -Help               Show this help message

Examples:
    .\run_tests.ps1                     # Run all tests
    .\run_tests.ps1 -Smoke              # Run smoke tests only
    .\run_tests.ps1 -Unit -Verbose      # Run unit tests with verbose output
    .\run_tests.ps1 -Clean -All         # Clean build and run all tests
    .\run_tests.ps1 -BuildOnly          # Just build the test executable

"@
}

# Function to check dependencies
function Test-Dependencies {
    Write-Info "Checking dependencies..."
    
    # Check for git submodules
    $JuceCMake = Join-Path $PluginDir "JUCE\CMakeLists.txt"
    if (-not (Test-Path $JuceCMake)) {
        Write-Info "JUCE submodule not found, initializing..."
        Set-Location $ProjectRoot
        git submodule update --init --recursive
        if ($LASTEXITCODE -ne 0) {
            Write-Error-Custom "Failed to initialize git submodules"
            exit 1
        }
    }
    
    # Check for MSBuild
    try {
        $msbuild = Get-Command msbuild -ErrorAction Stop
        Write-Info "Found MSBuild: $($msbuild.Source)"
    }
    catch {
        Write-Error-Custom "MSBuild not found. Please install Visual Studio 2022 or Build Tools."
        Write-Info "You can also run: winget install Microsoft.VisualStudio.2022.BuildTools"
        exit 1
    }
    
    # Check Visual Studio components
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstances = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64
        if (-not $vsInstances) {
            Write-Warning "Visual Studio C++ build tools may not be installed"
            Write-Info "Please ensure Visual Studio 2022 with C++ workload is installed"
        }
    }
}

# Function to clean build directories
function Clear-Builds {
    if ($Clean) {
        Write-Info "Cleaning build directories..."
        
        $TestBuildPath = Join-Path $BuildDir "x64"
        if (Test-Path $TestBuildPath) {
            Remove-Item $TestBuildPath -Recurse -Force
            Write-Info "Cleaned test build directory"
        }
        
        $ProjectorBuildPath = Join-Path $ProjectorBuildDir "x64"
        if (Test-Path $ProjectorBuildPath) {
            Remove-Item $ProjectorBuildPath -Recurse -Force
            Write-Info "Cleaned Projucer build directory"
        }
    }
}

# Function to build Projucer
function Build-Projucer {
    Write-Info "Building Projucer..."
    
    if ((Test-Path $ProjectorExecutable) -and -not $Clean) {
        Write-Info "Projucer already built, skipping..."
        return
    }
    
    Set-Location $ProjectorBuildDir
    
    msbuild Projucer.sln /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
    if ($LASTEXITCODE -ne 0) {
        Write-Error-Custom "Failed to build Projucer"
        exit 1
    }
    
    if (-not (Test-Path $ProjectorExecutable)) {
        Write-Error-Custom "Projucer executable not found after build"
        exit 1
    }
    
    Write-Success "Projucer built successfully"
}

# Function to generate test project
function New-TestProject {
    Write-Info "Generating test project..."
    
    Set-Location $ProjectRoot
    $JucerFile = Join-Path $TestDir "TremoloViola_Tests.jucer"
    
    & $ProjectorExecutable --resave $JucerFile
    if ($LASTEXITCODE -ne 0) {
        Write-Error-Custom "Failed to generate test project"
        exit 1
    }
    
    if (-not (Test-Path $BuildDir)) {
        Write-Error-Custom "Build directory not created"
        exit 1
    }
    
    Write-Success "Test project generated"
}

# Function to build tests
function Build-Tests {
    Write-Info "Building test application..."
    
    Set-Location $BuildDir
    
    $SolutionFile = "TremoloViola Tests.sln"
    if (-not (Test-Path $SolutionFile)) {
        Write-Error-Custom "Solution file not found: $SolutionFile"
        Get-ChildItem
        exit 1
    }
    
    msbuild $SolutionFile /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
    if ($LASTEXITCODE -ne 0) {
        Write-Error-Custom "Failed to build test application"
        exit 1
    }
    
    if (-not (Test-Path $TestExecutable)) {
        Write-Error-Custom "Test executable not found after build"
        Write-Info "Expected: $TestExecutable"
        Write-Info "Build directory contents:"
        Get-ChildItem (Join-Path $BuildDir "x64\Release") -Recurse
        exit 1
    }
    
    Write-Success "Test application built successfully"
}

# Function to create test source files
function Test-SourceFiles {
    Write-Info "Checking test source files..."
    
    # Create the Tests/Source directory if it doesn't exist
    $SourceDir = Join-Path $TestDir "Source"
    if (-not (Test-Path $SourceDir)) {
        New-Item -ItemType Directory -Path $SourceDir | Out-Null
    }
    
    # Check if test files exist
    $MainCpp = Join-Path $SourceDir "Main.cpp"
    if (-not (Test-Path $MainCpp)) {
        Write-Warning "Test source files not found in $SourceDir"
        Write-Warning "Please create the test files using the provided artifacts:"
        Write-Warning "- Main.cpp"
        Write-Warning "- SmokeTests.h and SmokeTests.cpp"
        Write-Warning "- TremoloLFOTests.h and TremoloLFOTests.cpp"
        Write-Warning "- PresetManagerTests.h and PresetManagerTests.cpp"
        Write-Warning "- PluginTests.h and PluginTests.cpp"
        return $false
    }
    
    return $true
}

# Function to run tests
function Invoke-Tests {
    if ($BuildOnly) {
        Write-Success "Build completed successfully (build-only mode)"
        return
    }
    
    Write-Info "Running tests..."
    
    # Prepare test arguments
    $TestArgs = @()
    
    switch ($TestType) {
        "smoke" { 
            $TestArgs += "--smoke"
            Write-Info "Running smoke tests..."
        }
        "unit" { 
            $TestArgs += "--unit"
            Write-Info "Running unit tests..."
        }
        "all" { 
            $TestArgs += "--all"
            Write-Info "Running all tests..."
        }
    }
    
    if ($Verbose) {
        $TestArgs += "--verbose"
    }
    
    # Change to test executable directory
    Set-Location (Split-Path $TestExecutable)
    
    # Run the tests
    & $TestExecutable @TestArgs
    $ExitCode = $LASTEXITCODE
    
    if ($ExitCode -eq 0) {
        Write-Success "All tests passed! 🎉"
    } else {
        Write-Error-Custom "Tests failed with exit code $ExitCode"
        exit $ExitCode
    }
}

# Main execution
function Main {
    # Show help if requested
    if ($Help) {
        Show-Help
        return
    }
    
    Write-Info "TremoloViola Plugin Test Runner for Windows"
    Write-Info "=========================================="
    
    try {
        Test-Dependencies
        Clear-Builds
        
        if (-not (Test-SourceFiles)) {
            Write-Error-Custom "Test source files are missing. Please create them first."
            exit 1
        }
        
        Build-Projucer
        New-TestProject
        Build-Tests
        Invoke-Tests
        
        Write-Success "Test run completed successfully!"
    }
    catch {
        Write-Error-Custom "An error occurred: $($_.Exception.Message)"
        Write-Error-Custom "Stack trace: $($_.ScriptStackTrace)"
        exit 1
    }
}

# Run main function
Main