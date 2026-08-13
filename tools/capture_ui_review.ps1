param(
    [string]$Round = "manual"
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$chromeCandidates = @(
    "C:\Program Files\Google\Chrome\Application\chrome.exe",
    "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    "C:\Program Files\Microsoft\Edge\Application\msedge.exe"
)
$browser = $chromeCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $browser) { throw "Chrome or Edge was not found." }

$preview = [System.Uri]::new((Resolve-Path (Join-Path $projectRoot "preview.html"))).AbsoluteUri + "?review=1"
$output = Join-Path $projectRoot "build\ui-review\$Round"
$profile = Join-Path $projectRoot "build\ui-review\browser-profile-$Round"
New-Item -ItemType Directory -Force -Path $output, $profile | Out-Null

foreach ($viewport in @("1920x1080", "1440x900", "1366x768", "1024x768", "768x1024", "430x932", "390x844")) {
    $dimensions = $viewport.Split("x")
    $screenshot = Join-Path $output "$viewport.png"
    & $browser --headless=new --user-data-dir=$profile --disable-gpu-sandbox --hide-scrollbars `
        --run-all-compositor-stages-before-draw --virtual-time-budget=4000 `
        --window-size=$($dimensions[0]),$($dimensions[1]) --screenshot=$screenshot $preview | Out-Null
}

Write-Output "UI screenshots: $output"
