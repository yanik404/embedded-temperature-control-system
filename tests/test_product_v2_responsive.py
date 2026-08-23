"""Validate the finished-product exterior and cutaway at all viewports."""

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


VIEWPORTS = ((1920, 1080), (1440, 900), (1366, 768), (768, 1024), (390, 844))
browser = next((candidate for candidate in review.CHROME if candidate.exists()), None)
if browser is None:
    print("Product V3 responsive test skipped: Chrome/Chromium not found")
    raise SystemExit(0)

with tempfile.TemporaryDirectory(prefix="becherhalter-product-v3-", ignore_cleanup_errors=True) as profile:
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

        for width, height in VIEWPORTS:
            cdp.call("Emulation.setDeviceMetricsOverride", {
                "width": width, "height": height, "deviceScaleFactor": 1,
                "mobile": width <= 430, "screenWidth": width, "screenHeight": height,
            })
            cdp.call("Page.navigate", {"url": (ROOT / "preview.html").as_uri() + "?scenario=heating"})
            time.sleep(.35)
            for view in ("exterior", "build"):
                expression = f"""(()=>{{
                    window.V4Twin.setProductView('{view}');
                    const expected=document.querySelector('[data-product-illustration="{view}"]');
                    const other=document.querySelector('[data-product-illustration="{'build' if view == 'exterior' else 'exterior'}"]');
                    const svg=expected.querySelector('svg');
                    const rect=svg.getBoundingClientRect();
                    const ids=[...document.querySelectorAll('[id]')].map(node=>node.id);
                    const panel=document.querySelector('.product-panel').getBoundingClientRect();
                    const stage=document.querySelector('.product-stage').getBoundingClientRect();
                    const stop=document.getElementById('stopButton').getBoundingClientRect();
                    return{{expected:getComputedStyle(expected).display,other:getComputedStyle(other).display,
                        width:rect.width,height:rect.height,left:rect.left,right:rect.right,
                        overflow:document.documentElement.scrollWidth-document.documentElement.clientWidth,
                        duplicateIds:ids.length-new Set(ids).size,view:document.body.dataset.productView,
                        panelTop:panel.top,stageTop:stage.top,panelLeft:panel.left,stageLeft:stage.left,stopHeight:stop.height}};
                }})()"""
                result = cdp.call("Runtime.evaluate", {"expression": expression, "returnByValue": True})
                metrics = result["result"]["value"]
                assert metrics["expected"] != "none" and metrics["other"] == "none", (width, height, view, metrics)
                assert metrics["view"] == view and metrics["duplicateIds"] == 0, (width, height, view, metrics)
                assert metrics["width"] > 240 and metrics["height"] > 280, (width, height, view, metrics)
                assert metrics["overflow"] <= 1, (width, height, view, metrics)
                if width <= 430:
                    assert metrics["panelTop"] < metrics["stageTop"], (width, height, view, metrics)
                    assert metrics["stopHeight"] >= 44, (width, height, view, metrics)
                elif width >= 1200:
                    assert metrics["stageLeft"] < metrics["panelLeft"], (width, height, view, metrics)
                if view == "build":
                    for mode in ("all", "thermal", "sensors", "electronics"):
                        mode_expression = f"""(()=>{{window.V4Twin.setBuildMode('{mode}');const control=document.querySelector('.build-mode-switch').getBoundingClientRect(),button=document.querySelector('[data-build-mode="{mode}"]');return{{mode:document.body.dataset.buildMode,pressed:button.getAttribute('aria-pressed'),display:getComputedStyle(document.querySelector('.build-mode-switch')).display,left:control.left,right:control.right}}}})()"""
                        mode_result = cdp.call("Runtime.evaluate", {"expression": mode_expression, "returnByValue": True})["result"]["value"]
                        assert mode_result["mode"] == mode and mode_result["pressed"] == "true", (width, height, mode, mode_result)
                        assert mode_result["display"] != "none" and mode_result["left"] >= 0 and mode_result["right"] <= width + 1, (width, height, mode, mode_result)
        cdp.call("Browser.close")
    finally:
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.terminate()

print("finished-product views responsive test passed: exterior and four interior modes at 1920, 1440, 1366, 768 and 390 px")
