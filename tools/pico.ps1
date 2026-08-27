param(
    [Parameter(Position = 0)]
    [ValidateSet("Configure", "Build", "Flash", "BuildFlash")]
    [string]$Action = "Build"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BuildDirectory = Join-Path $ProjectRoot "build\vscode"
$Uf2Path = Join-Path $BuildDirectory "temperature_control.uf2"

function Add-ExecutableDirectory {
    param([string]$Executable)
    if ($Executable) {
        $directory = Split-Path -Parent $Executable
        if (($env:Path -split ";") -notcontains $directory) { $env:Path = "$directory;$env:Path" }
    }
}

function Find-Executable {
    param([string]$Name, [string[]]$SearchRoots = @())
    $command = Get-Command $Name -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($command -and $command.Source) { return $command.Source }
    foreach ($root in $SearchRoots) {
        if (-not $root -or -not (Test-Path $root)) { continue }
        $match = Get-ChildItem -LiteralPath $root -Filter $Name -File -Recurse -ErrorAction SilentlyContinue |
            Sort-Object FullName -Descending | Select-Object -First 1
        if ($match) { return $match.FullName }
    }
    return $null
}

function Find-CMakePackageDirectory {
    param([string]$Package, [string[]]$SearchRoots)
    foreach ($root in $SearchRoots) {
        if (-not $root -or -not (Test-Path $root)) { continue }
        $match = Get-ChildItem -LiteralPath $root -Filter "${Package}Config.cmake" -File -Recurse -ErrorAction SilentlyContinue |
            Sort-Object FullName -Descending | Select-Object -First 1
        if ($match) { return $match.DirectoryName }
    }
    return $null
}

function Refresh-ToolPath {
    $searchRoots = @(
        (Join-Path ${env:ProgramFiles} "CMake"),
        (Join-Path ${env:ProgramFiles(x86)} "Arm GNU Toolchain arm-none-eabi"),
        (Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages")
    )
    Add-ExecutableDirectory (Find-Executable "cmake.exe" $searchRoots)
    Add-ExecutableDirectory (Find-Executable "ninja.exe" $searchRoots)
    Add-ExecutableDirectory (Find-Executable "arm-none-eabi-gcc.exe" $searchRoots)
    Add-ExecutableDirectory (Find-Executable "gcc.exe" $searchRoots)
    Add-ExecutableDirectory (Find-Executable "python.exe" @((Join-Path $env:LOCALAPPDATA "Programs\Python")))
}

function Get-PicoSdkPath {
    $candidates = @(
        $env:PICO_SDK_PATH,
        (Join-Path $ProjectRoot ".tools\pico-sdk-2.1.1"),
        (Join-Path $ProjectRoot "build\toolchains\pico-sdk-2.1.1"),
        (Join-Path $env:USERPROFILE ".pico-sdk\sdk\2.1.1"),
        (Join-Path $env:LOCALAPPDATA "Raspberry Pi\pico-sdk\2.1.1")
    )
    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path (Join-Path $candidate "external\pico_sdk_import.cmake"))) {
            return (Resolve-Path $candidate).Path
        }
    }
    throw "Pico SDK 2.1.1 wurde nicht gefunden. Fuehre zuerst den VS-Code-Task 'Laptop einrichten' aus."
}

function Invoke-Configure {
    Refresh-ToolPath
    $cmake = Find-Executable "cmake.exe" @((Join-Path ${env:ProgramFiles} "CMake"))
    if (-not $cmake) { throw "CMake fehlt. Fuehre zuerst 'Laptop einrichten' aus." }
    if (-not (Find-Executable "ninja.exe" @((Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages")))) { throw "Ninja fehlt. Fuehre zuerst 'Laptop einrichten' aus." }
    if (-not (Find-Executable "arm-none-eabi-gcc.exe" @((Join-Path ${env:ProgramFiles(x86)} "Arm GNU Toolchain arm-none-eabi")))) { throw "ARM GNU Toolchain fehlt. Fuehre zuerst 'Laptop einrichten' aus." }

    $sdk = Get-PicoSdkPath
    $arguments = @("--fresh", "-S", $ProjectRoot, "-B", $BuildDirectory, "-G", "Ninja", "-DPICO_BOARD=pico_w", "-DCMAKE_BUILD_TYPE=Release", "-DPICO_SDK_PATH=$sdk")
    $raspberryRoot = Join-Path $env:LOCALAPPDATA "Raspberry Pi"
    $pioasmDirectory = Find-CMakePackageDirectory "pioasm" @($raspberryRoot, (Join-Path $env:USERPROFILE ".pico-sdk"))
    $picotoolDirectory = Find-CMakePackageDirectory "picotool" @($raspberryRoot, (Join-Path $env:USERPROFILE ".pico-sdk"))
    if ($pioasmDirectory) { $arguments += "-Dpioasm_DIR=$pioasmDirectory" }
    if ($picotoolDirectory) { $arguments += "-Dpicotool_DIR=$picotoolDirectory" }

    Write-Host "Konfiguriere Pico-W-Firmware mit SDK $sdk"
    & $cmake @arguments
    if ($LASTEXITCODE -ne 0) { throw "CMake-Konfiguration fehlgeschlagen." }
}

function Invoke-Build {
    Refresh-ToolPath
    if (-not (Test-Path (Join-Path $BuildDirectory "CMakeCache.txt"))) { Invoke-Configure }
    $cmake = Find-Executable "cmake.exe" @((Join-Path ${env:ProgramFiles} "CMake"))
    Write-Host "Baue Firmware..."
    & $cmake --build $BuildDirectory --parallel 2
    if ($LASTEXITCODE -ne 0) { throw "Firmware-Build fehlgeschlagen." }
    if (-not (Test-Path $Uf2Path)) { throw "UF2 wurde nicht erzeugt: $Uf2Path" }
    Write-Host "Fertig: $Uf2Path"
}

function Invoke-Flash {
    if (-not (Test-Path $Uf2Path)) { throw "UF2 fehlt. Fuehre zuerst den Task 'Build' aus." }
    $picotool = Find-Executable "picotool.exe" @((Join-Path $env:LOCALAPPDATA "Raspberry Pi\picotool"), (Join-Path $env:USERPROFILE ".pico-sdk\picotool"))
    if ($picotool) {
        Write-Host "Flashe mit picotool..."
        & $picotool load -F -v $Uf2Path
        if ($LASTEXITCODE -ne 0) { throw "Picotool konnte die Firmware nicht laden." }
        & $picotool reboot -a
        if ($LASTEXITCODE -ne 0) { throw "Firmware geladen, aber automatischer Neustart fehlgeschlagen." }
        Write-Host "Firmware erfolgreich geladen und Pico neu gestartet."
        return
    }
    $bootVolume = Get-Volume -ErrorAction SilentlyContinue | Where-Object FileSystemLabel -eq "RPI-RP2" | Select-Object -First 1
    if ($bootVolume -and $bootVolume.DriveLetter) {
        Copy-Item -LiteralPath $Uf2Path -Destination "$($bootVolume.DriveLetter):\temperature_control.uf2" -Force
        Write-Host "Firmware erfolgreich auf das BOOTSEL-Laufwerk kopiert."
        return
    }
    throw "Picotool und BOOTSEL-Laufwerk wurden nicht gefunden. Pico mit gedrueckter BOOTSEL-Taste anschliessen und 'Flash' erneut starten."
}

switch ($Action) {
    "Configure" { Invoke-Configure }
    "Build" { Invoke-Build }
    "Flash" { Invoke-Flash }
    "BuildFlash" { Invoke-Configure; Invoke-Build; Invoke-Flash }
}
