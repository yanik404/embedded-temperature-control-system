"""Exercise the callout configurator and its SVG relationship in a real browser."""

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import capture_ui_review as review  # noqa: E402


browser = next((candidate for candidate in review.CHROME if candidate.exists()), None)
if browser is None:
    print("callout configurator browser test skipped: Chrome/Chromium not found")
    raise SystemExit(0)

with tempfile.TemporaryDirectory(prefix="becherhalter-callout-test-") as profile:
    port = review.free_port()
    process = subprocess.Popen(
        [str(browser), "--headless=new", f"--remote-debugging-port={port}",
         "--remote-allow-origins=*", f"--user-data-dir={profile}", "--no-first-run",
         "--no-default-browser-check", "--hide-scrollbars", "about:blank"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    try:
        targets = None
        for _ in range(80):
            try:
                with urllib.request.urlopen(f"http://127.0.0.1:{port}/json", timeout=.3) as response:
                    targets = json.load(response)
                if targets:
                    break
            except OSError:
                time.sleep(.1)
        assert targets, "browser DevTools endpoint did not start"
        page = next(target for target in targets if target.get("type") == "page")
        cdp = review.DevTools(page["webSocketDebuggerUrl"])
        cdp.call("Page.enable")
        cdp.call("Emulation.setDeviceMetricsOverride", {"width": 1440, "height": 900, "deviceScaleFactor": 1, "mobile": False})
        cdp.call("Page.navigate", {"url": (ROOT / "preview.html").as_uri() + "?scenario=minimal-system&component=peltier2"})
        time.sleep(.9)

        def evaluate(expression: str):
            result = cdp.call("Runtime.evaluate", {"expression": expression, "returnByValue": True})
            return result["result"].get("value")

        before = evaluate("""(()=>{const e=document.querySelector('[data-callout-entry="peltier2"]'),p=document.querySelector('[data-part="peltier2"]');return{status:e.querySelector('.callout-status').textContent,connected:e.classList.contains('connected'),hidden:p.classList.contains('disconnected'),lines:document.querySelectorAll('[data-callout-path]').length,details:!e.querySelector('.callout-detail').hidden}})()""")
        assert before == {"status": "+", "connected": False, "hidden": True, "lines": 0, "details": True}, before

        evaluate("document.querySelector('[data-callout-entry=\"peltier2\"] .callout-status').click()")
        time.sleep(.6)
        added = evaluate("""(()=>{const e=document.querySelector('[data-callout-entry="peltier2"]'),p=document.querySelector('[data-part="peltier2"]');return{status:e.querySelector('.callout-status').textContent,connected:e.classList.contains('connected'),visible:!p.classList.contains('disconnected'),detail:e.querySelector('[data-detail="live"]').textContent,count:document.getElementById('componentCount').textContent}})()""")
        assert added["status"] == "✓" and added["connected"] and added["visible"], added
        assert added["detail"] and added["count"].endswith("/ 19"), added

        evaluate("document.querySelector('[data-callout-entry=\"peltier2\"] .callout-remove').click()")
        time.sleep(.1)
        removed = evaluate("""(()=>{const e=document.querySelector('[data-callout-entry="peltier2"]'),p=document.querySelector('[data-part="peltier2"]');return{status:e.querySelector('.callout-status').textContent,connected:e.classList.contains('connected'),hidden:p.classList.contains('disconnected')}})()""")
        assert removed == {"status": "+", "connected": False, "hidden": True}, removed

        keyboard = evaluate("""(()=>{const part=document.querySelector('[data-focus="peltier1"]');part.dispatchEvent(new KeyboardEvent('keydown',{key:'Enter',bubbles:true}));const entry=document.querySelector('[data-callout-entry="peltier1"]');return{active:entry.classList.contains('active'),expanded:entry.querySelector('.callout-row').getAttribute('aria-expanded'),label:entry.querySelector('.callout-status').getAttribute('aria-label')}})()""")
        assert keyboard["active"] and keyboard["expanded"] == "true" and keyboard["label"], keyboard

        views = evaluate("""(()=>{const build=document.querySelector('[data-product-view="build"]');build.click();return{view:document.body.dataset.productView,pressed:build.getAttribute('aria-pressed'),exterior:getComputedStyle(document.querySelector('[data-product-illustration="exterior"]')).display,cutaway:getComputedStyle(document.querySelector('[data-product-illustration="build"]')).display}})()""")
        assert views == {"view": "build", "pressed": "true", "exterior": "none", "cutaway": "block"}, views

        cdp.call("Emulation.setDeviceMetricsOverride", {"width": 390, "height": 844, "deviceScaleFactor": 1, "mobile": True, "screenWidth": 390, "screenHeight": 844})
        mobile_lines = evaluate("window.V4Twin.drawLines();document.querySelectorAll('[data-callout-path]').length")
        assert mobile_lines == 0, mobile_lines
        cdp.call("Browser.close")
    finally:
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.terminate()

print("callout configurator browser test passed: view switch, focus line, add, details, remove, keyboard, mobile")
