param(
    [string]$Round = "manual"
)

$ErrorActionPreference = "Stop"
$python = "C:\Users\yanik\AppData\Local\Programs\Python\Python313\python.exe"
if (-not (Test-Path -LiteralPath $python)) { $python = "python" }
& $python (Join-Path $PSScriptRoot "capture_ui_review.py") --round $Round
if ($LASTEXITCODE -ne 0) { throw "UI screenshot capture failed." }
