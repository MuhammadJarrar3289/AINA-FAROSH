# Urdu Poetry Studio — POC (Qt/QML, Windows)

This is a minimal POC for the Urdu Poetry Studio app (Qt 6 + QML). It demonstrates:
- Loading an Urdu/Nastaliq font
- Right-to-left poetry editing with line-break preservation
- Modern animated QML UI
- Export of the current page to `exported_poem.pdf`

Important: this POC expects you to supply a Nastaliq-capable font file. For licensing reasons no proprietary fonts are bundled.

Prerequisites (Windows)
1. Install Qt 6 (MSVC 2019/2022) via the Qt installer or use MSYS2/MinGW (example uses MSVC).
2. Install CMake >= 3.16
3. Have Visual Studio (MSVC) installed if using MSVC toolchain.

Project setup
1. Create a project folder and copy the files from this POC into it.
2. Place a Nastaliq-capable TTF/OTF into `fonts/` and update `resources.qrc` to list the font file name (or name it `NotoNastaliqUrdu-Regular.ttf`).
3. Create a build directory:
   mkdir build
   cd build
   cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64/lib/cmake"
   cmake --build . --config Release

Running
- Run the built executable. Use the Export PDF button to create `exported_poem.pdf` in the exe directory.

Next steps (after you verify POC)
- Replace raster-export with vector text PDF pipeline (use QPdfWriter with proper font embedding and drawing text via QPainter with layout).
- Integrate HarfBuzz directly for low-level shaping control if needed (Qt uses HarfBuzz internally but direct control may be necessary for Kashida/kerning options).
- Add a document model and template engine, master pages, and book builder UI.
- Add a font-license inspector and optional font installer.

If you want, I can:
- Turn this into a full repo commit/branch layout (you said you will create the repo — give me owner/repo) and prepare a branch `poc/qt-nastaliq` with these files ready to push and CI build instructions for Windows.
- Extend the PDF export to embed vector text and selectable text in PDF (recommended next).
- Swap raster approach for a layout-based PDF export that preserves selectable text.

Tell me:
- Confirm Dark theme (or Light), and whether you want me to prepare the branch `poc/qt-nastaliq` ready to commit to your repo (please share owner/repo).
