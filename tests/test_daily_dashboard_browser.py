"""Exercise the simple daily dashboard in the local browser preview."""

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
    print("Daily dashboard browser test skipped: Chrome/Chromium not found")
    raise SystemExit(0)

with tempfile.TemporaryDirectory(prefix="becherhalter-daily-", ignore_cleanup_errors=True) as profile:
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
        cdp.call("Page.navigate", {"url": (ROOT / "preview.html").as_uri() + "?scenario=ready"})
        result = cdp.call("Runtime.evaluate", {"expression": """new Promise(resolve=>{
          setTimeout(()=>{
            PreviewDriver.setCup(false);
            setTimeout(()=>{
                document.getElementById('dailyCup').click();
              setTimeout(()=>{
                const manual={cup:V3UI.runtime.current.cup_manual,text:document.getElementById('dailyCup').textContent,
                  status:document.getElementById('dailyCup').dataset.status,
                  colour:getComputedStyle(document.getElementById('dailyCup')).color};
                document.getElementById('dailyCup').click();
                setTimeout(()=>{
                  const stoppedManual={cup:V3UI.runtime.current.cup_manual,state:V3UI.runtime.current.state};
                  PreviewDriver.setCup(true);
                  const target=document.getElementById('dailyTarget');target.value='20,0';target.dispatchEvent(new Event('input'));
                  document.getElementById('dailySetpoint').click();
                  setTimeout(()=>{
                    document.getElementById('dailyStart').click();
                    setTimeout(()=>{
                      const cooling={state:V3UI.runtime.current.state,power:V3UI.runtime.current.power,
                        liquid:getComputedStyle(document.body).getPropertyValue('--cup-state-color').trim(),
                        heatBar:document.getElementById('limitHeatBar').querySelector('i').getBoundingClientRect().width,
                        coolBar:document.getElementById('limitCoolBar').querySelector('i').getBoundingClientRect().width,
                        coolTrack:document.getElementById('limitCoolBar').getBoundingClientRect().width};
                      document.getElementById('dailyStop').click();
                      setTimeout(()=>resolve({manual,stoppedManual,cooling,stopped:V3UI.runtime.current.state}),120);
                    },180);
                  },120);
                },120);
              },500);
            },120);
          },500);
        })""", "awaitPromise": True, "returnByValue": True})["result"]["value"]
        assert result["manual"]["cup"] is True and "manuell" in result["manual"]["text"], result
        assert result["manual"]["status"] == "manual" and result["manual"]["colour"] == "rgb(198, 109, 60)", result
        assert result["stoppedManual"] == {"cup": False, "state": "AUS"}, result
        assert result["cooling"]["state"] == "KUEHLEN" and result["cooling"]["power"] < 0, result
        assert result["cooling"]["liquid"] == "#67bde0", result
        assert result["cooling"]["coolBar"] >= result["cooling"]["coolTrack"] - 1, result
        assert result["stopped"] == "BEREIT", result
        cdp.call("Browser.close")
    finally:
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.terminate()

print("daily dashboard browser test passed: manual cup, stop, cooling and power bars")
