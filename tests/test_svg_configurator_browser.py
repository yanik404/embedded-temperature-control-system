"""Exercise the direct one-click SVG configurator in a real browser."""

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
    print("SVG configurator browser test skipped: Chrome/Chromium not found")
    raise SystemExit(0)

with tempfile.TemporaryDirectory(prefix="becherhalter-svg-test-") as profile:
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
        cdp.call("Page.navigate", {"url": (ROOT / "preview.html").as_uri() + "?scenario=minimal-system"})
        time.sleep(.8)

        def evaluate(expression: str):
            result = cdp.call("Runtime.evaluate", {"expression": expression, "returnByValue": True})
            return result["result"].get("value")

        before = evaluate("""(()=>{const b=document.querySelector('[data-component-control="peltier2"]'),p=document.querySelector('[data-part="peltier2"]');return{button:b.textContent.trim(),connected:b.classList.contains('connected'),hidden:p.classList.contains('disconnected')}})()""")
        assert before == {"button": "+Peltier 2", "connected": False, "hidden": True}, before

        evaluate("document.querySelector('[data-component-control=\"peltier2\"]').click()")
        time.sleep(.6)
        added = evaluate("""(()=>{const b=document.querySelector('[data-component-control="peltier2"]'),p=document.querySelector('[data-part="peltier2"]');return{connected:b.classList.contains('connected'),visible:!p.classList.contains('disconnected'),status:document.getElementById('componentConnected').textContent}})()""")
        assert added == {"connected": True, "visible": True, "status": "JA"}, added

        evaluate("document.querySelector('[data-component-control=\"peltier2\"]').click()")
        removed = evaluate("""(()=>{const b=document.querySelector('[data-component-control="peltier2"]'),p=document.querySelector('[data-part="peltier2"]');return{connected:b.classList.contains('connected'),hidden:p.classList.contains('disconnected'),status:document.getElementById('componentConnected').textContent}})()""")
        assert removed == {"connected": False, "hidden": True, "status": "NEIN"}, removed
        cdp.call("Browser.close")
    finally:
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.terminate()

print("direct SVG configurator browser test passed: add, connected status, remove")
