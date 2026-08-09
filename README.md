# Urdu Poetry Studio — POC (Qt/QML, Windows)

This is a minimal POC for the Urdu Poetry Studio app (Qt 6 + QML). It demonstrates:
- Loading an Urdu/Nastaliq font from the local fonts/ folder (preferred) or system fonts
- Right-to-left poetry editing with line-break preservation
- Modern animated QML UI with Dark/Light theme toggle
- Export of the current page to a high-resolution `exported_poem.pdf`

Fonts
- The app now loads fonts placed into the `fonts/` directory next to the executable. This lets you add fonts without rebuilding the QRC.
- I added a helper script `scripts/add_noto_nastaliq.ps1` which will attempt to download the free NotoNastaliqUrdu-Regular.ttf into `fonts/` (Windows PowerShell). If the download fails, manually add a TTF/OTF into `fonts/`.

Important: do not commit licensed/proprietary fonts into this repository unless you own redistribution rights. Keep licensed fonts locally in your machine's fonts folder or in a private storage.

Prerequisites (Windows)
1. Install Qt 6 (MSVC 2019/2022) via the Qt installer or use MSYS2/MinGW (example uses MSVC).
2. Install CMake >= 3.16
3. Have Visual Studio (MSVC) installed if using MSVC toolchain.

Project setup
1. Clone your repo and switch to branch `poc/qt-nastaliq` (already pushed).
2. Option A (recommended): run the helper script to get Noto Nastaliq locally:
   - Open PowerShell in the repo root and run:
       scripts\add_noto_nastaliq.ps1
   - Confirm a font file was downloaded into `fonts/`.

3. Option B: manually copy a Nastaliq-capable TTF/OTF into `fonts/`.

4. Build the project:
   - Create a build directory:
       mkdir build
       cd build
       cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64/lib/cmake"
       cmake --build . --config Release

Running
- Run the built executable. The app will search for fonts in ./fonts/ and load any .ttf/.otf found. Use the theme toggle (moon/sun) to switch Dark/Light. Use the Export PDF button to create `exported_poem.pdf` in the exe directory.

Next steps
- Optionally I can add a settings panel to manage project fonts (enable/disable, view license, mark as embed-allowed).
- Next technical priority: replace raster-export with vector text PDF pipeline (recommended) so exported PDFs are selectable and fonts are embedded where license permits.
