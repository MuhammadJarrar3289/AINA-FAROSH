# Urdu Poetry Studio — POC (Qt/QML)

This branch demonstrates a Nastaliq-friendly editor and selective font-embedding PDF export.

## What it includes
- QML editor UI (`qml/Main.qml`, `qml/PoetryEditor.qml`) with RTL Urdu editing.
- Font manager panel (`qml/FontManager.qml`) inside the right inspector (Properties / Fonts tabs).
- Local font loading from `./fonts/` at startup.
- FreeType-based embedding detection from OS/2 `fsType` (`src/FontInspector.*`).
- Per-project manifest (`project.manifest.json`) managed by `src/ProjectManager.*`.
- PDF export (`src/Backend.*`) that:
  - keeps vector/selectable text when embedding is allowed,
  - rasterizes only disallowed-font runs,
  - writes a JSON export report next to the PDF.

## Build requirements
- CMake >= 3.16
- Qt6 (Core, Quick, Gui, Widgets, PrintSupport)
- FreeType development package
- C++ toolchain (MSVC recommended on Windows)

## Quick run
1. Put a Nastaliq-capable font in `./fonts/` (or run `scripts/add_noto_nastaliq.ps1` on Windows).
2. Configure and build (example):
   ```bash
   cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64/lib/cmake"
   cmake --build build --config Release
   ```
3. Run the app:
   - Fonts tab auto-loads `project.manifest.json` (if present) and auto-scans `./fonts/`.
   - Toggle embed override(s); overrides auto-save to `project.manifest.json`.
   - Export PDF from header button.

## Expected export artifacts
- `exported_poem.pdf`
- `exported_poem_export_report.json` (embedded vs rasterized font decisions)

## Reviewer checks
1. Confirm build finds Qt6 + PrintSupport + FreeType.
2. Confirm manifest updates when embed override is toggled.
3. Confirm export writes both PDF and JSON report.
4. Verify selectable/searchable text where embedding is allowed.
5. Verify disallowed-font runs are rasterized and listed in report.
6. Verify missing font fallback behavior is noted in report.
