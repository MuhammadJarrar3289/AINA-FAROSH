#!/usr/bin/env pwsh
<#
  add_noto_nastaliq.ps1
  PowerShell helper: download Noto Nastaliq Urdu TTF into ./fonts/
  This script attempts to download the font from the Google Fonts GitHub repository raw URL.
  If the URL fails, open the script and change $url to a working raw file URL for the desired font.
#>

$fontsDir = Join-Path -Path (Get-Location) -ChildPath "fonts"
if (-not (Test-Path $fontsDir)) {
    New-Item -ItemType Directory -Path $fontsDir | Out-Null
}

$url = "https://github.com/googlefonts/noto-fonts/raw/main/phaseIII_only/Arabic/NotoNastaliqUrdu/NotoNastaliqUrdu-Regular.ttf"
$out = Join-Path -Path $fontsDir -ChildPath "NotoNastaliqUrdu-Regular.ttf"

Write-Host "Attempting to download Noto Nastaliq Urdu from:`n  $url`ninto:`n  $out`n"

try {
    Invoke-WebRequest -Uri $url -OutFile $out -UseBasicParsing -ErrorAction Stop
    Write-Host "Downloaded font to $out"
} catch {
    Write-Warning "Download failed. The URL may be unavailable."
    Write-Host "Please manually download a Nastaliq-capable TTF/OTF and place it into the ./fonts/ folder, then run the app again."
}

Write-Host "Available fonts in ./fonts/:"
Get-ChildItem -Path $fontsDir -Include *.ttf, *.otf -File | ForEach-Object { Write-Host " - " $_.Name }
