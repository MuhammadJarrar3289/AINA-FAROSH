#!/usr/bin/env bash
# setup.sh — Urdu Poetry Studio (Linux/macOS)
#
# One-shot setup: checks prerequisites, fetches a Nastaliq font, configures
# CMake, builds the app, and launches it (unless --no-run is passed).
#
# Usage:
#   ./setup.sh
#   ./setup.sh --qt-dir /opt/Qt/6.7.2/gcc_64 --no-run

set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

QT_DIR=""
CONFIG="Release"
RUN=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --qt-dir) QT_DIR="$2"; shift 2 ;;
    --config) CONFIG="$2"; shift 2 ;;
    --no-run) RUN=0; shift ;;
    *) echo "Unknown arg: $1"; exit 1 ;;
  esac
done

fail() { echo ""; echo "ERROR: $1" >&2; exit 1; }

echo "== Urdu Poetry Studio — setup =="

# 1. cmake
command -v cmake >/dev/null 2>&1 || fail "cmake not found. Install it (e.g. 'sudo apt install cmake' or 'brew install cmake') and re-run."
echo "[ok] cmake found: $(cmake --version | head -n1)"

# 2. Locate Qt6 if not given
if [[ -z "$QT_DIR" ]]; then
  for base in "$HOME/Qt" "/opt/Qt" "/usr/lib/qt6" "/usr/local/opt/qt6"; do
    if [[ -d "$base" ]]; then
      found=$(find "$base" -maxdepth 3 -type d -iname "gcc_64" -o -iname "clang_64" 2>/dev/null | head -n1 || true)
      if [[ -n "$found" ]]; then QT_DIR="$found"; break; fi
    fi
  done
  if [[ -z "$QT_DIR" ]] && command -v qmake6 >/dev/null 2>&1; then
    QT_DIR="$(dirname "$(dirname "$(command -v qmake6)")")"
  fi
  if [[ -z "$QT_DIR" ]]; then
    fail "Qt6 not found automatically. Install Qt 6 (e.g. 'sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev' on Ubuntu, or 'brew install qt6' on macOS, or the Qt online installer), then re-run with --qt-dir /path/to/Qt/6.x/gcc_64."
  fi
  echo "[ok] Auto-detected Qt at: $QT_DIR"
else
  [[ -d "$QT_DIR" ]] || fail "Given --qt-dir '$QT_DIR' does not exist."
fi

# 3. Freetype
if ! pkg-config --exists freetype2 2>/dev/null; then
  echo "[warn] Freetype not detected via pkg-config. Install it, e.g.:"
  echo "         Ubuntu/Debian: sudo apt install libfreetype-dev"
  echo "         macOS:         brew install freetype"
  echo "       CMake will fail below if it's missing."
fi

# 4. Fetch a Nastaliq font if none present
mkdir -p fonts
if [[ -z "$(find fonts -maxdepth 1 -iname '*.ttf' -o -iname '*.otf' 2>/dev/null)" ]]; then
  echo "No font found in ./fonts — attempting download of Noto Nastaliq Urdu..."
  URL="https://github.com/googlefonts/noto-fonts/raw/main/phaseIII_only/Arabic/NotoNastaliqUrdu/NotoNastaliqUrdu-Regular.ttf"
  if command -v curl >/dev/null 2>&1; then
    curl -fsSL "$URL" -o "fonts/NotoNastaliqUrdu-Regular.ttf" || echo "[warn] Download failed — manually place a .ttf/.otf into ./fonts/"
  elif command -v wget >/dev/null 2>&1; then
    wget -q "$URL" -O "fonts/NotoNastaliqUrdu-Regular.ttf" || echo "[warn] Download failed — manually place a .ttf/.otf into ./fonts/"
  else
    echo "[warn] Neither curl nor wget found — manually place a .ttf/.otf into ./fonts/"
  fi
else
  echo "[ok] Font(s) already present in ./fonts"
fi

# 5. Configure
echo "== Configuring (CMake) =="
cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build" -DCMAKE_PREFIX_PATH="$QT_DIR" -DCMAKE_BUILD_TYPE="$CONFIG" \
  || fail "CMake configure failed. See errors above (commonly: Freetype or Qt6 components not found)."

# 6. Build
echo "== Building ($CONFIG) =="
cmake --build "$ROOT_DIR/build" --config "$CONFIG" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)" \
  || fail "Build failed. See compiler errors above."

# 7. Locate executable and copy fonts next to it
EXE_PATH="$(find "$ROOT_DIR/build" -maxdepth 3 -type f -iname "UrduPoetryStudio" | head -n1 || true)"
[[ -n "$EXE_PATH" ]] || fail "Build succeeded but the UrduPoetryStudio executable was not found under build/."
EXE_DIR="$(dirname "$EXE_PATH")"
mkdir -p "$EXE_DIR/fonts"
cp -f fonts/* "$EXE_DIR/fonts/" 2>/dev/null || true

echo ""
echo "== Build complete =="
echo "Executable: $EXE_PATH"

if [[ "$RUN" -eq 1 ]]; then
  echo "Launching app..."
  (cd "$EXE_DIR" && "./$(basename "$EXE_PATH")" &)
else
  echo "Skipped launch (--no-run). Run it yourself with:"
  echo "  \"$EXE_PATH\""
fi
