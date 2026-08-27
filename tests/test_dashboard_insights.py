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

        setpoint_expression = """new Promise(resolve=>{
          PreviewDriver.apply('ready');
          const input=document.getElementById('targetInput');
          document.getElementById('targetUp').click();
          document.getElementById('targetUp').click();
          document.getElementById('targetUp').click();
          document.getElementById('targetDown').click();
          setTimeout(()=>{
            const draft=input.value;
            document.getElementById('setpointButton').click();
            setTimeout(()=>{
              const confirmed=document.getElementById('heroSetpoint').textContent;
              document.getElementById('startButton').click();
              setTimeout(()=>resolve({
                unlocked:document.body.classList.contains('control-unlocked'),disabled:input.disabled,
                draft,confirmed,input:input.value,target:document.getElementById('heroSetpoint').textContent,
                state:document.getElementById('systemState').textContent,
                runtimeTarget:V3UI.runtime.current.setpoint,
                message:document.getElementById('controlMessage').textContent
              }),350);
            },100);
          },350);
        })"""
        setpoint_state = cdp.call("Runtime.evaluate", {"expression": setpoint_expression, "awaitPromise": True,
                                                        "returnByValue": True})["result"]["value"]
        assert setpoint_state["unlocked"] and not setpoint_state["disabled"], setpoint_state
        assert setpoint_state["draft"] == "46,0", setpoint_state
        assert setpoint_state["confirmed"] == "46,0" and setpoint_state["input"] == "46,0", setpoint_state
        assert setpoint_state["target"] == "46,0" and setpoint_state["runtimeTarget"] == 46, setpoint_state
        assert setpoint_state["state"] == "AUFHEIZEN" and "nur Demo-Daten" in setpoint_state["message"], setpoint_state

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

        colour_expression = """(()=>{
          const colours={};
          for(const scenario of ['ready','heating','cooling','holding','error']){
            PreviewDriver.apply(scenario);
            colours[scenario]={
              cup:getComputedStyle(document.body).getPropertyValue('--cup-state-color').trim(),
              loop:getComputedStyle(document.querySelector('.heat-step strong')).color,
              analysis:getComputedStyle(document.querySelector('.analysis-heading small')).color,
              sensor:getComputedStyle(document.querySelector('.sensor-step strong')).color,
              sensorIcon:getComputedStyle(document.querySelector('.loop-sensor path')).stroke,
              metric:getComputedStyle(document.querySelector('.hero-metrics dl div:nth-child(3) dd')).color,
              livePower:getComputedStyle(document.getElementById('powerOutput')).color,
              activeBorder:getComputedStyle(document.querySelector('.timeline-layer button.active')).borderTopColor
            };
          }
          PreviewDriver.apply('ready');
          return colours;
        })()"""
        colours = cdp.call("Runtime.evaluate", {"expression": colour_expression,
                                                  "returnByValue": True})["result"]["value"]
        expected_accents = {
            "ready": ("#76aec4", "rgb(118, 174, 196)"),
            "heating": ("#ed8549", "rgb(237, 133, 73)"),
            "cooling": ("#67bde0", "rgb(118, 174, 196)"),
            "holding": ("#71c393", "rgb(113, 195, 147)"),
            "error": ("#eb6868", "rgb(235, 104, 104)"),
        }
        for state, (cup, accent) in expected_accents.items():
            assert colours[state]["cup"] == cup, colours
            assert all(colours[state][key] == accent for key in
                       ("loop", "analysis", "sensor", "sensorIcon", "metric", "livePower", "activeBorder")), colours

        feedback_geometry = cdp.call("Runtime.evaluate", {"expression": """(()=>{
          const route=document.querySelector('.feedback-route').getBoundingClientRect();
          const sum=document.querySelector('.compare-connector>span').getBoundingClientRect();
          const sensor=document.querySelector('.sensor-step strong').getBoundingClientRect();
          return {routeLeft:route.left,routeRight:route.right,sumLeft:sum.left,sensorRight:sensor.right};
        })()""", "returnByValue": True})["result"]["value"]
        assert feedback_geometry["routeLeft"] < feedback_geometry["sumLeft"], feedback_geometry
        assert feedback_geometry["routeRight"] > feedback_geometry["sensorRight"], feedback_geometry

        theme_expression = """(()=>{
          V3UI.setThemeMode('auto');PreviewDriver.apply('day-mode');
          const autoDay={theme:document.body.dataset.theme,label:document.getElementById('ambientMode').textContent,
            source:document.getElementById('ambientSource').textContent,icon:getComputedStyle(document.querySelector('.sun-icon')).display};
          PreviewDriver.apply('night-mode');
          const autoNight={theme:document.body.dataset.theme,label:document.getElementById('ambientMode').textContent,
            source:document.getElementById('ambientSource').textContent,icon:getComputedStyle(document.querySelector('.moon-icon')).display};
          document.querySelector('[data-theme-choice="day"]').click();PreviewDriver.apply('night-mode');
          const manualDay={theme:document.body.dataset.theme,mode:document.body.dataset.themeMode,
            source:document.getElementById('ambientSource').textContent,pressed:document.querySelector('[data-theme-choice="day"]').getAttribute('aria-pressed')};
          document.querySelector('[data-theme-choice="auto"]').click();PreviewDriver.apply('day-mode');
          return {autoDay,autoNight,manualDay,restored:{theme:document.body.dataset.theme,mode:document.body.dataset.themeMode}};
        })()"""
        themes = cdp.call("Runtime.evaluate", {"expression": theme_expression,
                                                 "returnByValue": True})["result"]["value"]
        assert themes["autoDay"] == {"theme": "day", "label": "TAGMODUS", "source": "AUTO · 86 %", "icon": "block"}, themes
        assert themes["autoNight"] == {"theme": "night", "label": "NACHTMODUS", "source": "AUTO · 8 %", "icon": "block"}, themes
        assert themes["manualDay"] == {"theme": "day", "mode": "day", "source": "MANUELL", "pressed": "true"}, themes
        assert themes["restored"] == {"theme": "day", "mode": "auto"}, themes

        cooling_expression = """new Promise(resolve=>{
          PreviewDriver.apply('ready');
          const input=document.getElementById('targetInput');input.value='20,0';input.dispatchEvent(new Event('input'));
          document.getElementById('setpointButton').click();setTimeout(()=>{
            document.getElementById('startButton').click();setTimeout(()=>resolve({
              state:document.body.dataset.state,label:document.getElementById('systemState').textContent,
              target:V3UI.runtime.current.setpoint,power:V3UI.runtime.current.power,
              peltier:document.getElementById('loopPeltierState').textContent,
              cup:getComputedStyle(document.body).getPropertyValue('--cup-state-color').trim()
            }),250);
          },100);
        })"""
        cooling = cdp.call("Runtime.evaluate", {"expression": cooling_expression, "awaitPromise": True,
                                                  "returnByValue": True})["result"]["value"]
        assert cooling["state"] == "KUEHLEN" and cooling["label"] == "KÜHLEN", cooling
        assert cooling["target"] == 20 and -20 <= cooling["power"] < 0, cooling
        assert cooling["peltier"] == "AKTIV · KÜHLEN" and cooling["cup"] == "#67bde0", cooling

        cdp.call("Runtime.evaluate", {"expression": "PreviewDriver.apply('ready')"})

        cup_expression = """new Promise(resolve=>{
          PreviewDriver.setCup(false);
          setTimeout(()=>{
            const absent={dataset:document.body.dataset.cup,
              state:document.body.dataset.state,
              badge:document.getElementById('cupPresenceTitle').textContent,
              badgeStatus:document.getElementById('cupPresence').dataset.status,
              signal:document.getElementById('cupPresenceSignal').textContent,
              empty:getComputedStyle(document.querySelector('.product-v3-empty-holder')).opacity,
              exterior:getComputedStyle(document.querySelector('.product-v3-cup-state')).opacity,
              cutaway:getComputedStyle(document.querySelector('.cutaway-live-cup')).opacity,
              loop:getComputedStyle(document.querySelector('.loop-product')).opacity,
              label:document.getElementById('plantState').textContent,
              start:document.getElementById('startButton').disabled,
              reason:document.getElementById('startBlockReason').textContent};
            PreviewDriver.setCup(true);
            setTimeout(()=>resolve({absent,present:{dataset:document.body.dataset.cup,
              state:document.body.dataset.state,
              badge:document.getElementById('cupPresenceTitle').textContent,
              badgeStatus:document.getElementById('cupPresence').dataset.status,
              signal:document.getElementById('cupPresenceSignal').textContent,
              empty:getComputedStyle(document.querySelector('.product-v3-empty-holder')).opacity,
              exterior:getComputedStyle(document.querySelector('.product-v3-cup-state')).opacity,
              cutaway:getComputedStyle(document.querySelector('.cutaway-live-cup')).opacity,
              loop:getComputedStyle(document.querySelector('.loop-product')).opacity,
              start:document.getElementById('startButton').disabled}}),550);
          },550);
        })"""
        cup = cdp.call("Runtime.evaluate", {"expression": cup_expression, "awaitPromise": True,
                                             "returnByValue": True})["result"]["value"]
        assert cup["absent"] == {"dataset": "absent", "state": "AUS", "badge": "BECHER FEHLT", "badgeStatus": "absent",
                                  "signal": "S_DETECT · GP13 RAW 0", "empty": "1", "exterior": "0",
                                  "cutaway": "0", "loop": "0.22", "label": "Kein Becher erkannt",
                                  "start": True, "reason": "Kein Becher erkannt"}, cup
        assert cup["present"] == {"dataset": "present", "state": "BEREIT", "badge": "BECHER ERKANNT", "badgeStatus": "present",
                                   "signal": "S_DETECT · GP13 RAW 1", "empty": "0", "exterior": "1",
                                   "cutaway": "1", "loop": "1", "start": False}, cup

        removal_expression = """new Promise(resolve=>{
          PreviewDriver.apply('heating');PreviewDriver.setCup(false);setTimeout(()=>resolve({
            state:document.body.dataset.state,fault:document.getElementById('cupPresenceTitle').textContent,
            power:V3UI.runtime.current.power,start:document.getElementById('startButton').disabled,
            reason:document.getElementById('safetyReason').textContent
          }),250);
        })"""
        removal = cdp.call("Runtime.evaluate", {"expression": removal_expression, "awaitPromise": True,
                                                  "returnByValue": True})["result"]["value"]
        assert removal["state"] == "FEHLER" and removal["fault"] == "BECHER ENTFERNT", removal
        assert removal["power"] == 0 and removal["start"] and "Becher" in removal["reason"], removal

        cup_guard_expression = """new Promise(resolve=>{
          PreviewDriver.command('stop',false);const stopped={state:document.body.dataset.state,
            allowed:V3UI.runtime.current.start_allowed,power:V3UI.runtime.current.power};
          const accepted=PreviewDriver.command('start','preview-session');setTimeout(()=>resolve({stopped,accepted,
            state:document.body.dataset.state,power:V3UI.runtime.current.power,
            reason:document.getElementById('startBlockReason').textContent}),80);
        })"""
        cup_guard = cdp.call("Runtime.evaluate", {"expression": cup_guard_expression, "awaitPromise": True,
                                                   "returnByValue": True})["result"]["value"]
        assert cup_guard["stopped"] == {"state": "AUS", "allowed": False, "power": 0}, cup_guard
        assert not cup_guard["accepted"] and cup_guard["state"] == "AUS" and cup_guard["power"] == 0, cup_guard
        assert "Becher" in cup_guard["reason"], cup_guard

        viewport_expression = """new Promise(resolve=>{
          PreviewDriver.openViewport('mobile');setTimeout(()=>{
            const dialog=document.querySelector('.preview-viewport-dialog');
            const frame=document.querySelector('.preview-viewport-frame');
            const shell=document.querySelector('.preview-device-frame');
            resolve({open:dialog.open,width:frame.style.width,height:frame.style.height,
              mobile:shell.classList.contains('mobile'),title:document.querySelector('.preview-viewport-title').textContent});
          },120);
        })"""
        viewport = cdp.call("Runtime.evaluate", {"expression": viewport_expression, "awaitPromise": True,
                                                  "returnByValue": True})["result"]["value"]
        assert viewport == {"open": True, "width": "390px", "height": "844px", "mobile": True,
                            "title": "HANDY-ANSICHT · 390 × 844"}, viewport
        cdp.call("Browser.close")
    finally:
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.terminate()

print("dashboard insight checks passed: controls, live cup presence, state colours, metrics and recovery")
