# Running Urdu Poetry Studio

This is a native Qt 6 / QML desktop app (C++). It is not a web app, so it must
be compiled and run on your own machine — it can't run inside a browser or a
chat sandbox.

## Quick start

### Windows
Open **PowerShell**, `cd` into the project folder, then:
```powershell
powershell -ExecutionPolicy Bypass -File setup.ps1
```
This checks for CMake and Qt6, downloads a Nastaliq font if you don't have
one, configures the project, builds it, and launches the app.

### Linux / macOS
```bash
chmod +x setup.sh
./setup.sh
```
Same steps as above, adapted for `apt`/`brew`-installed Qt.

Both scripts accept `-QtDir` / `--qt-dir` if Qt isn't auto-detected, and
`-NoRun` / `--no-run` if you just want it built, not launched.

## Prerequisites (installed once)

| Tool | Windows | Linux (Ubuntu/Debian) | macOS |
|---|---|---|---|
| CMake ≥ 3.16 | [cmake.org](https://cmake.org/download/) or `winget install Kitware.CMake` | `sudo apt install cmake` | `brew install cmake` |
| Qt 6.2+ (Quick, Gui, Widgets, PrintSupport) | [Qt online installer](https://www.qt.io/download-qt-installer), MSVC kit | `sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev` | `brew install qt6` |
| Freetype | bundled via [vcpkg](https://github.com/microsoft/vcpkg) (`vcpkg install freetype:x64-windows`) | `sudo apt install libfreetype-dev` | `brew install freetype` |
| C++ compiler | Visual Studio 2019/2022 (MSVC) | `sudo apt install build-essential` | Xcode Command Line Tools |

On Windows specifically, Freetype for the MSVC toolchain usually needs vcpkg:
```powershell
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install freetype:x64-windows
$env:VCPKG_ROOT = (Resolve-Path .\vcpkg)
```
`setup.ps1` will pick up `$env:VCPKG_ROOT` automatically if it's set before
you run it.

## Manual build (if you'd rather not use the scripts)

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x.x/<kit>"   # e.g. C:/Qt/6.7.2/msvc2019_64
cmake --build . --config Release
```
The executable is `UrduPoetryStudio` (or `UrduPoetryStudio.exe` on Windows)
inside the build directory. Make sure a `fonts/` folder containing at least
one Nastaliq-capable `.ttf`/`.otf` sits **next to the executable** — the app
scans that folder at startup. Run `scripts/add_noto_nastaliq.ps1` (Windows)
or let `setup.sh` fetch one automatically, or drop your own font in by hand.

## What the app does once running

- Type or paste Urdu poetry into the right-to-left editor (a sample poem
  loads by default).
- Switch fonts and size from the **Properties** tab on the right.
- Check embedding permissions per font under the **Fonts** tab — this reads
  each font's `OS/2 fsType` bit via FreeType to decide whether it's legally
  embeddable in an exported PDF, and lets you override that decision.
- Toggle dark/light theme with the moon/sun button.
- **Export PDF** produces `exported_poem.pdf` next to the executable — vector
  text where the font allows embedding, rasterized text where it doesn't —
  plus a `*_export_report.json` explaining the decision made for each font.

## Troubleshooting

- **"Qt6 not found"** — pass the exact kit folder, e.g.
  `-QtDir "C:\Qt\6.7.2\msvc2019_64"` / `--qt-dir /opt/Qt/6.7.2/gcc_64`.
- **"Freetype not found" during CMake configure** — install it per the table
  above (Windows: via vcpkg, then set `VCPKG_ROOT`).
- **App launches but Urdu text looks wrong (boxes/garbled)** — no
  Nastaliq-capable font is in `fonts/` next to the executable; add one.
- **Export PDF button does nothing / errors in console** — check that the
  QML object hierarchy wasn't renamed; the exporter looks for objects named
  `poetryCanvas` and `poetryEditor`, which are already wired up in
  `qml/Main.qml`.
