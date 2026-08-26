"""Verify the debounced S_DETECT path from GP13 to safety and dashboard."""

from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
config = (ROOT / "include" / "config.h").read_text(encoding="utf-8")
buttons = (ROOT / "src" / "buttons.c").read_text(encoding="utf-8")
app = (ROOT / "src" / "app.c").read_text(encoding="utf-8")
safety = (ROOT / "src" / "safety.c").read_text(encoding="utf-8")
webserver = (ROOT / "src" / "webserver.c").read_text(encoding="utf-8")
display = (ROOT / "src" / "display.c").read_text(encoding="utf-8")
dashboard = (ROOT / "ui-v3" / "src" / "experience.js").read_text(encoding="utf-8")

for token in (
    "CUP_DETECT_ACTIVE_LEVEL     1u",
    "CUP_DETECT_INSERT_MS        100u",
    "CUP_DETECT_REMOVE_MS        50u",
):
    assert token in config
assert "cup_detector_update" in buttons and "buttons_cup_raw_level" in buttons
assert "gpio_pull_down(PIN_S_DETECT)" in buttons
assert "[DETECT] GP13 raw=" in buttons and "[DETECT] Stabil:" in buttons
assert "status.cup_detected = buttons_cup_detected();" in app
assert "status.cup_switch_raw = buttons_cup_raw_level();" in app
assert "status.state == SYSTEM_READY && !safety_can_start(&status)" in app
assert "status.state == SYSTEM_OFF && !manual_off && safety_can_start(&status)" in app
assert "ERROR_CUP_REMOVED" in safety and "!status->cup_detected" in safety
assert "cup_switch_raw" in webserver and "cup_active_level" in webserver
assert display.count("BECHER: OK") == 2 and display.count("BECHER: FEHLT") == 2
assert "cupPresenceTitle" in dashboard and "BECHER ENTFERNT" in dashboard

compiler = shutil.which("cc") or shutil.which("gcc")
if compiler:
    with tempfile.TemporaryDirectory(prefix="cup-detector-") as directory:
        output = Path(directory) / ("cup_detector.exe" if os.name == "nt" else "cup_detector")
        subprocess.run(
            [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-Iinclude",
             "tests/test_cup_detector.c", "src/cup_detector.c", "-o", str(output)],
            cwd=ROOT, check=True,
        )
        subprocess.run([str(output)], cwd=ROOT, check=True)

print("cup detect contract passed: debounced GP13, safety gate, OLED and dashboard")
