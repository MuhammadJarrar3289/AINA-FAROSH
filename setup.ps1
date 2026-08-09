#!/usr/bin/env pwsh
<#
  setup.ps1 — Urdu Poetry Studio (Windows)

  One-shot setup: checks prerequisites, fetches a Nastaliq font, configures
  CMake, builds the app, and (unless -NoRun is passed) launches it.

  Usage:
    powershell -ExecutionPolicy Bypass -File setup.ps1
    # or, from a Qt/MSVC developer prompt:
    ./setup.ps1
    ./setup.ps1 -QtDir "C:\Qt\6.7.2\msvc2019_64" -NoRun
#>

param(
    [string]$QtDir = "",
    [string]$Config = "Release",
    [switch]$NoRun
)

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
Set-Location $root

function Fail($msg) {
    Write-Host ""
    Write-Host "ERROR: $msg" -ForegroundColor Red
    exit 1
}

Write-Host "== Urdu Poetry Studio — setup ==" -ForegroundColor Cyan

# 1. Check CMake
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Fail "cmake not found on PATH. Install it from https://cmake.org/download/ (or 'winget install Kitware.CMake') and re-run."
}
Write-Host "[ok] cmake found: $(cmake --version | Select-Object -First 1)"

# 2. Locate Qt6 if not given
if ($QtDir -eq "") {
    $candidates = Get-ChildItem "C:\Qt" -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^6\.' } |
        ForEach-Object { Get-ChildItem $_.FullName -Directory -ErrorAction SilentlyContinue } |
        Where-Object { $_.Name -match 'msvc' }
    if ($candidates) {
        $QtDir = $candidates[0].FullName
        Write-Host "[ok] Auto-detected Qt at: $QtDir"
    } else {
        Fail "Qt6 not found under C:\Qt. Install Qt 6 (MSVC kit) from https://www.qt.io/download-qt-installer, then re-run with -QtDir pointing at e.g. 'C:\Qt\6.7.2\msvc2019_64'."
    }
} else {
    if (-not (Test-Path $QtDir)) { Fail "Given -QtDir '$QtDir' does not exist." }
}

# 3. Check Freetype (bundled with Qt's mingw kit, but MSVC kits typically need vcpkg)
$freetypeHint = ""
if ($env:VCPKG_ROOT) {
    $freetypeHint = "-DCMAKE_TOOLCHAIN_FILE=`"$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`""
    Write-Host "[ok] VCPKG_ROOT detected, will use vcpkg toolchain for Freetype."
} else {
    Write-Host "[warn] VCPKG_ROOT not set. If CMake fails to find Freetype, install it via vcpkg:" -ForegroundColor Yellow
    Write-Host "         git clone https://github.com/microsoft/vcpkg && .\vcpkg\bootstrap-vcpkg.bat"
    Write-Host "         .\vcpkg\vcpkg install freetype:x64-windows"
    Write-Host "         `$env:VCPKG_ROOT = (Resolve-Path .\vcpkg)"
    Write-Host "       then re-run this script."
}

# 4. Fetch a Nastaliq font if none present
$fontsDir = Join-Path $root "fonts"
New-Item -ItemType Directory -Force -Path $fontsDir | Out-Null
$hasFont = Get-ChildItem $fontsDir -Include *.ttf, *.otf -File -ErrorAction SilentlyContinue
if (-not $hasFont) {
    Write-Host "No font found in ./fonts — attempting download of Noto Nastaliq Urdu..."
    & "$root\scripts\add_noto_nastaliq.ps1"
} else {
    Write-Host "[ok] Font(s) present in ./fonts: $($hasFont.Name -join ', ')"
}

# 5. Configure
$buildDir = Join-Path $root "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
Write-Host "== Configuring (CMake) ==" -ForegroundColor Cyan
$cmakeArgs = @("-S", $root, "-B", $buildDir, "-DCMAKE_PREFIX_PATH=$QtDir")
if ($freetypeHint -ne "") { $cmakeArgs += $freetypeHint }
cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { Fail "CMake configure failed. See errors above (commonly: Freetype not found — see the vcpkg hint above)." }

# 6. Build
Write-Host "== Building ($Config) ==" -ForegroundColor Cyan
cmake --build $buildDir --config $Config
if ($LASTEXITCODE -ne 0) { Fail "Build failed. See compiler errors above." }

# 7. Copy fonts folder next to the built executable so the app can find them at runtime
$exeCandidates = Get-ChildItem $buildDir -Recurse -Filter "UrduPoetryStudio.exe" -ErrorAction SilentlyContinue
if (-not $exeCandidates) { Fail "Build succeeded but UrduPoetryStudio.exe was not found under $buildDir." }
$exePath = $exeCandidates[0].FullName
$exeDir = Split-Path $exePath -Parent
$destFonts = Join-Path $exeDir "fonts"
New-Item -ItemType Directory -Force -Path $destFonts | Out-Null
Copy-Item "$fontsDir\*" $destFonts -Force -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "== Build complete ==" -ForegroundColor Green
Write-Host "Executable: $exePath"

if (-not $NoRun) {
    Write-Host "Launching app..."
    Start-Process -FilePath $exePath -WorkingDirectory $exeDir
} else {
    Write-Host "Skipped launch (-NoRun). Run it yourself with:"
    Write-Host "  & `"$exePath`""
}
