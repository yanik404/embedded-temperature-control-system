$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$SdkPath = Join-Path $ProjectRoot ".tools\pico-sdk-2.1.1"

function Refresh-Path {
    $machinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $env:Path = "$machinePath;$userPath"
}

function Install-WingetPackage {
    param([string]$Command, [string]$PackageId, [string]$Title)
    $installedCommand = Get-Command $Command -ErrorAction SilentlyContinue
    if ($installedCommand -and $installedCommand.Source -notlike "*\\WindowsApps\\python.exe") {
        & $installedCommand.Source --version *> $null
        if ($LASTEXITCODE -eq 0) { Write-Host "OK: $Title"; return }
    }
    Write-Host "Installiere $Title..."
    & winget install --id $PackageId -e --source winget --silent --accept-package-agreements --accept-source-agreements
    if ($LASTEXITCODE -ne 0) { throw "$Title konnte nicht installiert werden ($PackageId)." }
    Refresh-Path
}

if (-not (Get-Command winget.exe -ErrorAction SilentlyContinue)) { throw "Windows Package Manager (winget) fehlt. Installiere zuerst 'App-Installer' aus dem Microsoft Store." }

Install-WingetPackage "git.exe" "Git.Git" "Git"
Install-WingetPackage "cmake.exe" "Kitware.CMake" "CMake"
Install-WingetPackage "ninja.exe" "Ninja-build.Ninja" "Ninja"
Install-WingetPackage "python.exe" "Python.Python.3.13" "Python"
Install-WingetPackage "arm-none-eabi-gcc.exe" "Arm.GnuArmEmbeddedToolchain" "ARM GNU Toolchain"
Install-WingetPackage "gcc.exe" "BrechtSanders.WinLibs.POSIX.UCRT" "nativer C/C++-Compiler"

if (Get-Command code.cmd -ErrorAction SilentlyContinue) {
    Write-Host "Installiere/aktualisiere die Raspberry-Pi-Pico-Erweiterung fuer VS Code..."
    & code.cmd --install-extension raspberry-pi.raspberry-pi-pico --force
}

if (-not (Test-Path (Join-Path $SdkPath "external\pico_sdk_import.cmake"))) {
    New-Item -ItemType Directory -Path (Split-Path -Parent $SdkPath) -Force | Out-Null
    Write-Host "Lade Pico SDK 2.1.1 inklusive Submodule..."
    & git clone --branch 2.1.1 --depth 1 --recurse-submodules --shallow-submodules https://github.com/raspberrypi/pico-sdk.git $SdkPath
    if ($LASTEXITCODE -ne 0) { throw "Pico SDK konnte nicht geladen werden." }
} else {
    Write-Host "OK: Pico SDK 2.1.1 ist bereits vorhanden."
}

# Pico SDK 2.1.1 benoetigt diese Standard-Header bei neuen MinGW-Versionen explizit.
foreach ($relativePath in @("tools\pioasm\output_format.h", "tools\pioasm\pio_types.h")) {
    $headerPath = Join-Path $SdkPath $relativePath
    $content = [IO.File]::ReadAllText($headerPath)
    if ($content -notmatch '#include <cstdint>') {
        $content = $content.Replace('#include "pio_enums.h"', "#include `"pio_enums.h`"`r`n#include <cstdint>")
        [IO.File]::WriteAllText($headerPath, $content, (New-Object Text.UTF8Encoding($false)))
    }
}

Write-Host "Pruefe die Build-Konfiguration..."
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "pico.ps1") Configure
if ($LASTEXITCODE -ne 0) { throw "Einrichtung abgeschlossen, aber die Build-Konfiguration ist fehlgeschlagen." }
Write-Host "Laptop ist eingerichtet. Ab jetzt genuegen die VS-Code-Tasks 'Build' oder 'Build & Flash'."
