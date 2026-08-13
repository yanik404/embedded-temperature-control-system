#!/usr/bin/env python3
"""Audit visible UI text/hotspot collisions at the required review viewports."""

from __future__ import annotations

import argparse
import json
import tempfile
from pathlib import Path

import capture_ui_review as review


ROOT = Path(__file__).resolve().parents[1]
MODES = {
    "product": "review=product",
    "control": "review=loop",
    "live": "review=live",
}


def audit(browser: Path, width: int, height: int, query: str, profile: Path) -> dict:
    port = review.free_port()
    process = review.subprocess.Popen(
        [str(browser), "--headless=new", f"--remote-debugging-port={port}",
         "--remote-allow-origins=*", f"--user-data-dir={profile}", "--disable-gpu-sandbox",
         "--hide-scrollbars", "--no-first-run", "--no-default-browser-check", "about:blank"],
        stdout=review.subprocess.DEVNULL, stderr=review.subprocess.DEVNULL,
    )
    try:
        targets = None
        for _ in range(80):
            try:
                with review.urllib.request.urlopen(f"http://127.0.0.1:{port}/json", timeout=.3) as response:
                    targets = json.load(response)
                if targets:
                    break
            except OSError:
                review.time.sleep(.1)
        page = next(target for target in targets if target.get("type") == "page")
        cdp = review.DevTools(page["webSocketDebuggerUrl"])
        cdp.call("Page.enable")
        cdp.call("Emulation.setDeviceMetricsOverride", {"width": width, "height": height,
                 "deviceScaleFactor": 1, "mobile": width <= 430, "screenWidth": width, "screenHeight": height})
        cdp.call("Page.navigate", {"url": (ROOT / "preview.html").as_uri() + "?" + query})
        expression = """new Promise(resolve=>{const start=performance.now(),wait=()=>{
          if(document.body?.classList.contains('ui-ready'))setTimeout(()=>resolve((()=>{
            const selectors='.section-heading,.product-summary,.twin-hotspot,.component-lens,.loop-step,.loop-product,.temperature-hero,.live-facts,.timeline-layer,.control-panel,.safety-summary,.technology-section summary';
            const effectiveOpacity=n=>{let value=1;for(let p=n;p;p=p.parentElement){const s=getComputedStyle(p);if(s.display==='none'||s.visibility==='hidden')return 0;value*=Number(s.opacity);}return value};
            const nodes=[...document.querySelectorAll(selectors)].filter(n=>{const r=n.getBoundingClientRect();return effectiveOpacity(n)>.3&&r.width>2&&r.height>2&&r.bottom>0&&r.top<innerHeight&&r.right>0&&r.left<innerWidth});
            const rect=n=>{const r=n.getBoundingClientRect();return{x:r.x,y:r.y,w:r.width,h:r.height,name:n.dataset.component||n.id||n.className}};
            const boxes=nodes.map(rect),collisions=[];
            for(let i=0;i<boxes.length;i++)for(let j=i+1;j<boxes.length;j++){if(nodes[i].contains(nodes[j])||nodes[j].contains(nodes[i]))continue;const a=boxes[i],b=boxes[j],x=Math.min(a.x+a.w,b.x+b.w)-Math.max(a.x,b.x),y=Math.min(a.y+a.h,b.y+b.h)-Math.max(a.y,b.y);if(x>3&&y>3)collisions.push([a.name,b.name,Math.round(x*y)]);}
            const overflow=[...document.querySelectorAll('.twin-hotspot,.component-lens,.loop-step,.loop-product')].filter(n=>{const s=getComputedStyle(n),r=n.getBoundingClientRect(),verticalPixels=Math.min(r.bottom,innerHeight)-Math.max(r.top,0),meaningfullyVisible=verticalPixels>Math.min(44,r.height*.35)&&r.right>0&&r.left<innerWidth;return meaningfullyVisible&&s.display!=='none'&&s.visibility!=='hidden'&&(r.left<0||r.right>innerWidth)}).map(n=>n.dataset.component||n.id||n.className);
            return{collisions,overflow,scrollY:Math.round(scrollY),readyMs:document.documentElement.dataset.uiReadyMs||null,scrollWidth:document.documentElement.scrollWidth};
          })()),250);else if(performance.now()-start>3500)resolve({timeout:true});else requestAnimationFrame(wait)};wait()})"""
        result = cdp.call("Runtime.evaluate", {"expression": expression, "awaitPromise": True, "returnByValue": True})["result"]["value"]
        cdp.call("Browser.close")
        return result
    finally:
        try:
            process.wait(timeout=2)
        except review.subprocess.TimeoutExpired:
            process.terminate()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=ROOT / "build" / "ui-review" / "layout-audit.json")
    parser.add_argument("--modes", help="comma-separated subset of modes")
    parser.add_argument("--viewports", help="comma-separated subset such as 1920x1080,390x844")
    args = parser.parse_args()
    browser = next((candidate for candidate in review.CHROME if candidate.exists()), None)
    if not browser:
        raise SystemExit("Chrome or Edge was not found")
    report = {}
    with tempfile.TemporaryDirectory(prefix="becherhalter-layout-") as directory:
        selected_modes = args.modes.split(",") if args.modes else list(MODES)
        selected_viewports = review.VIEWPORTS
        if args.viewports:
            selected_viewports = tuple(tuple(int(value) for value in item.split("x", 1)) for item in args.viewports.split(","))
        for mode in selected_modes:
            query = MODES[mode]
            report[mode] = {}
            for width, height in selected_viewports:
                key = f"{width}x{height}"
                result = audit(browser, width, height, query, Path(directory) / f"{mode}-{key}")
                report[mode][key] = result
                print(f"{mode:12} {key:10} collisions={len(result.get('collisions', []))} overflow={len(result.get('overflow', []))}", flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
