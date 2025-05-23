#!/usr/bin/env powershell

<#
.SYNOPSIS
    LawnKeeper - Build automation for Win32 project
.DESCRIPTION
    Clean, build, format, and manage Win32 application project
.EXAMPLE
    .\lawnkeeper.ps1           # Default: clean and build
    .\lawnkeeper.ps1 -Clean    # Clean only
    .\lawnkeeper.ps1 -Format   # Format source code
    .\lawnkeeper.ps1 -All      # Format, generate, and build
#>

param(
    [switch]$Help, [switch]$Clean, [switch]$Build, [switch]$Rebuild,
    [switch]$Generate, [switch]$Format, [switch]$CheckFormat, [switch]$All,
    [string]$Generator = "",
    [ValidateSet("Debug", "Release")][string]$Config = "Release"
)

# Configuration
$BuildDir = "build"
$ProjectName = "DXMiniApp"
$ClangFormatPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\clang-format.exe"
$SourceExtensions = @("*.cpp", "*.c", "*.h", "*.hpp", "*.cc", "*.cxx", "*.hxx")

# Output helpers
function Log($msg, $color = "White") { Write-Host "🌿 $msg" -ForegroundColor $color }
function Success($msg) { Write-Host "✅ $msg" -ForegroundColor Green }
function Error($msg) { Write-Host "❌ $msg" -ForegroundColor Red }
function Warn($msg) { Write-Host "⚠️  $msg" -ForegroundColor Yellow }

function Show-Help {
    Write-Host @"
🌿 LawnKeeper - Win32 Project Build Script

USAGE: .\lawnkeeper.ps1 [action] [options]

ACTIONS:
    -Help         Show this help
    -Clean        Clean build directory
    -Build        Build project
    -Rebuild      Clean and build (default)
    -Generate     Generate project files only
    -Format       Format source code
    -CheckFormat  Check code formatting
    -All          Format + generate + build

OPTIONS:
    -Generator    CMake generator ("Visual Studio 17 2022", "Ninja", etc.)
    -Config       Debug or Release (default: Release)

EXAMPLES:
    .\lawnkeeper.ps1                    # Clean and build
    .\lawnkeeper.ps1 -Build -Config Debug
    .\lawnkeeper.ps1 -Format
    .\lawnkeeper.ps1 -All
"@ -ForegroundColor Cyan
}

function Test-Prerequisites {
    if (-not (Test-Path "CMakeLists.txt")) { Error "CMakeLists.txt not found"; return $false }
    if (-not (Test-Path "src")) { Error "src/ directory not found"; return $false }
    return $true
}

function Test-ClangFormat {
    if (-not (Test-Path $ClangFormatPath)) {
        Error "clang-format not found at: $ClangFormatPath"
        Warn "Install Visual Studio 2022 with C++ tools"
        return $false
    }
    return $true
}

function Get-SourceFiles {
    $files = @()
    foreach ($dir in @("src", "include")) {
        if (Test-Path $dir) {
            foreach ($ext in $SourceExtensions) {
                $files += Get-ChildItem -Path $dir -Filter $ext -Recurse
            }
        }
    }
    return $files
}

function Invoke-Clean {
    Log "Cleaning project..." "Cyan"
    if (Test-Path $BuildDir) {
        try {
            Remove-Item -Recurse -Force $BuildDir
            Success "Build directory cleaned"
            return $true
        }
        catch { Error "Failed to clean: $_"; return $false }
    }
    Warn "Build directory doesn't exist"
    return $true
}

function Get-Generator {
    if ($Generator) { return $Generator }

    Log "Auto-detecting generator..." "Cyan"
    if (Get-Command "ninja" -EA SilentlyContinue) { return "Ninja" }
    if (Get-Command "mingw32-make" -EA SilentlyContinue) { return "MinGW Makefiles" }

    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInfo = & $vsWhere -latest -property displayName
        if ($vsInfo -match "2022") { return "Visual Studio 17 2022" }
        if ($vsInfo -match "2019") { return "Visual Studio 16 2019" }
    }
    return ""
}

function Invoke-Generate {
    Log "Generating project files..." "Cyan"

    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
        Log "Created build directory"
    }

    Push-Location $BuildDir
    try {
        $gen = Get-Generator
        Log "Using generator: $gen"

        $args = @("..")
        if ($gen) { $args += @("-G", $gen) }
        
        $output = & cmake $args 2>&1
        if ($LASTEXITCODE -ne 0) { 
            Error "Configuration failed:"
            $output | ForEach-Object { Write-Host $_ -ForegroundColor Red }
            throw "Configuration failed" 
        }
        
        Success "Project files generated"
        return $true
    }
    catch { Error "Generation failed: $_"; return $false }
    finally { Pop-Location }
}

function Invoke-Build {
    Log "Building project ($Config)..." "Cyan"

    if (-not (Test-Path "$BuildDir/CMakeCache.txt")) {
        Warn "Project files not found, generating..."
        if (-not (Invoke-Generate)) { return $false }
    }

    Push-Location $BuildDir
    try {
        $output = & cmake --build . --config $Config 2>&1
        if ($LASTEXITCODE -ne 0) { 
            Error "Build failed:"
            $output | ForEach-Object { Write-Host $_ -ForegroundColor Red }
            throw "Build failed" 
        }
        
        Success "Build completed"

        # Show executable location
        $exePaths = @("bin\$ProjectName.exe", "$Config\$ProjectName.exe", "$ProjectName.exe")
        foreach ($path in $exePaths) {
            if (Test-Path $path) { Log "Executable: $PWD\$path" "Green"; break }
        }
        return $true
    }
    catch { Error "Build failed: $_"; return $false }
    finally { Pop-Location }
}

function Invoke-Format {
    Log "Formatting source code..." "Cyan"
    if (-not (Test-ClangFormat)) { return $false }

    $files = Get-SourceFiles
    if (-not $files) { Warn "No source files found"; return $true }

    $formatted = 0
    $errors = 0

    foreach ($file in $files) {
        try {
            & $ClangFormatPath -i $file.FullName
            if ($LASTEXITCODE -eq 0) { $formatted++ } else { Error "Failed: $($file.Name)"; $errors++ }
        }
        catch { Error "Error formatting $($file.Name): $_"; $errors++ }
    }

    if ($errors -eq 0) {
        Success "Formatted $formatted files"
        return $true
    } else {
        Error "Failed: $errors files, Succeeded: $formatted files"
        return $false
    }
}

function Invoke-CheckFormat {
    Log "Checking source code formatting..." "Cyan"
    if (-not (Test-ClangFormat)) { return $false }

    $files = Get-SourceFiles
    if (-not $files) { Warn "No source files found"; return $true }

    $badFiles = @()
    foreach ($file in $files) {
        try {
            & $ClangFormatPath --dry-run --Werror $file.FullName 2>$null
            if ($LASTEXITCODE -ne 0) { $badFiles += $file.Name }
        }
        catch { Error "Error checking $($file.Name): $_"; return $false }
    }

    if (-not $badFiles) {
        Success "All $($files.Count) files correctly formatted"
        return $true
    } else {
        Error "Incorrectly formatted files:"
        $badFiles | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
        Warn "Run: .\lawnkeeper.ps1 -Format"
        return $false
    }
}

function Get-Action {
    $actions = @($Help, $Clean, $Build, $Rebuild, $Generate, $Format, $CheckFormat, $All)
    $actionNames = @("help", "clean", "build", "rebuild", "generate", "format", "check-format", "all")

    $activeCount = ($actions | Where-Object { $_ }).Count
    if ($activeCount -gt 1) {
        Error "Multiple actions specified. Use only one."
        Show-Help; exit 1
    }

    for ($i = 0; $i -lt $actions.Count; $i++) {
        if ($actions[$i]) { return $actionNames[$i] }
    }
    return "rebuild"  # default
}

# Main execution
Log "🌿 LawnKeeper - Win32 Project Build Script" "Cyan"

$action = Get-Action
Log "Action: $action | Config: $Config"

if ($action -ne "help" -and -not (Test-Prerequisites)) { exit 1 }

$success = switch ($action) {
    "help" { Show-Help; $true }
    "clean" { Invoke-Clean }
    "format" { Invoke-Format }
    "check-format" { Invoke-CheckFormat }
    "build" { Invoke-Build }
    "rebuild" { (Invoke-Clean) -and (Invoke-Build) }
    "generate" { Invoke-Generate }
    "all" { (Invoke-Format) -and (Invoke-Generate) -and (Invoke-Build) }
    default { Error "Unknown action: $action"; Show-Help; $false }
}

if (-not $success) { exit 1 }
if ($action -ne "help") { Success "LawnKeeper finished! 🌿" }