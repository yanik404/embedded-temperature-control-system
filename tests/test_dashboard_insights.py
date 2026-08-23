"""Exercise browser-local engineering calculations and connection scenarios."""

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


production = (ROOT / "ui" / "dist" / "dashboard.production.html").read_text(encoding="utf-8")
preview = (ROOT / "preview.html").read_text(encoding="utf-8")
for token in (
    "HARDWARE & VERBINDUNGEN", "ΔT Sensoren", "Session-Energie",
    "PROGNOSE NICHT VERFÜGBAR", "PI-Diagnose", "Systemmeldungen",
    "Verbindung wiederhergestellt", "PRÄSENTIEREN",
):
    assert token in production, f"missing engineering UI token: {token}"
assert "supply_voltage:12" not in production, "demo supply voltage leaked into production"
assert "supply_voltage:12" in preview, "preview needs a declared demo voltage for power calculation"

browser = next((candidate for candidate in review.CHROME if candidate.exists()), None)
if browser is None:
    print("dashboard insight browser test skipped: Chrome/Chromium not found")
    raise SystemExit(0)

with tempfile.TemporaryDirectory(prefix="becherhalter-insights-", ignore_cleanup_errors=True) as profile:
    port = review.free_port()
    process = subprocess.Popen(
        [str(browser), "--headless=new", f"--remote-debugging-port={port}",
         "--remote-allow-origins=*", f"--user-data-dir={profile}", "--no-first-run",
         "--no-default-browser-check", "about:blank"],
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
        cdp.call("Page.navigate", {"url": (ROOT / "preview.html").as_uri() + "?scenario=heating"})
        time.sleep(.45)

        expression = """(()=>{
          const m=window.V3Metrics,now=Date.now();
          const status={temp1_ok:true,temp2_ok:true,temperature1:33.6,temperature2:32.4,
            current1_ok:true,current2_ok:true,current1:2.8,current2:2.7,supply_voltage:12,
            state:'AUFHEIZEN',temperature:32.4,setpoint:45};
          const points=[0,1,2,3,4].map(i=>({time:now-(20-i*5)*1000,temperature:30+i/6,valid:true}));
          const stepPoints=[
            {time:now,temperature:20,valid:true},{time:now+10000,temperature:23,valid:true},
            {time:now+20000,temperature:39,valid:true},{time:now+30000,temperature:47,valid:true},
            {time:now+40000,temperature:50,valid:true},{time:now+50000,temperature:50.5,valid:true},
            {time:now+60000,temperature:50.1,valid:true}
          ];
          const step=m.stepResponseMetrics(stepPoints,20,50,now,now+60000);
          return {delta:m.deltaTemperature(status),current:m.totalCurrent(status),power:m.electricalPower(status),
            missingPower:m.electricalPower({...status,supply_voltage:NaN}),slope:m.temperatureSlope(points,now),
            eta:m.estimateEtaMinutes(status,2),invalidDelta:m.deltaTemperature({...status,temp2_ok:false}),step};
        })()"""
        values = cdp.call("Runtime.evaluate", {"expression": expression, "returnByValue": True})["result"]["value"]
        assert abs(values["delta"] - 1.2) < 1e-6, values
        assert abs(values["current"] - 5.5) < 1e-6, values
        assert abs(values["power"] - 66.0) < 1e-6, values
        assert values.get("missingPower") is None and values.get("invalidDelta") is None, values
        assert values["slope"] > 1.9 and values["slope"] < 2.1, values
        assert abs(values["eta"] - 6.3) < 1e-6, values
        assert values["step"]["settled"] is True, values
        assert values["step"]["riseMs"] == 20000 and values["step"]["t63Ms"] == 20000, values
        assert abs(values["step"]["overshoot"] - .5) < 1e-6, values
        assert values["step"]["modelHint"] == "PT1 ALS STARTMODELL PLAUSIBEL", values

        scenario_expression = """(()=>{
          PreviewDriver.apply('disconnect');
          const offline={live:document.body.dataset.live,start:document.getElementById('startButton').disabled,
            banner:document.getElementById('connectionTitle').textContent};
          PreviewDriver.apply('recovery');
          return {offline,live:document.body.dataset.live,recovery:document.getElementById('recoveryToast').hidden,
            hardware:document.querySelectorAll('[data-hardware]').length};
        })()"""
        states = cdp.call("Runtime.evaluate", {"expression": scenario_expression, "returnByValue": True})["result"]["value"]
        assert states["offline"]["live"] == "false" and states["offline"]["start"], states
        assert states["live"] == "true" and states["recovery"] is False, states
        assert states["hardware"] >= 17, states
        cdp.call("Browser.close")
    finally:
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.terminate()

print("dashboard insight checks passed: live metrics, step response, disconnect and recovery")
