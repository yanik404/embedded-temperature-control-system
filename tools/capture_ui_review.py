#!/usr/bin/env python3
"""Capture exact CSS viewports with Chrome DevTools Protocol and no Python packages."""

from __future__ import annotations

import argparse
import base64
import json
import os
import secrets
import socket
import struct
import subprocess
import time
import urllib.request
from pathlib import Path
from urllib.parse import urlparse


ROOT = Path(__file__).resolve().parents[1]
VIEWPORTS = ((1920, 1080), (1440, 900), (1366, 768), (1024, 768), (768, 1024), (430, 932), (390, 844))
CHROME = (
    Path(r"C:\Program Files\Google\Chrome\Application\chrome.exe"),
    Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"),
    Path(r"C:\Program Files\Microsoft\Edge\Application\msedge.exe"),
    Path("/usr/bin/google-chrome"),
    Path("/usr/bin/chromium"),
    Path("/usr/bin/chromium-browser"),
)


class DevTools:
    def __init__(self, url: str):
        parsed = urlparse(url)
        self.sock = socket.create_connection((parsed.hostname, parsed.port), timeout=20)
        key = base64.b64encode(os.urandom(16)).decode()
        request = (
            f"GET {parsed.path} HTTP/1.1\r\nHost: {parsed.hostname}:{parsed.port}\r\n"
            f"Upgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n"
            "Sec-WebSocket-Version: 13\r\n\r\n"
        )
        self.sock.sendall(request.encode("ascii"))
        response = b""
        while b"\r\n\r\n" not in response:
            response += self.sock.recv(4096)
        if b" 101 " not in response.split(b"\r\n", 1)[0]:
            raise RuntimeError(response.decode("latin1", errors="replace"))
        self.index = 0

    def _exact(self, count: int) -> bytes:
        data = b""
        while len(data) < count:
            chunk = self.sock.recv(count - len(data))
            if not chunk:
                raise ConnectionError("Chrome closed the DevTools socket")
            data += chunk
        return data

    def _send(self, payload: bytes, opcode: int = 1) -> None:
        mask = secrets.token_bytes(4)
        length = len(payload)
        header = bytearray([0x80 | opcode])
        if length < 126:
            header.append(0x80 | length)
        elif length < 65536:
            header.append(0x80 | 126)
            header.extend(struct.pack("!H", length))
        else:
            header.append(0x80 | 127)
            header.extend(struct.pack("!Q", length))
        header.extend(mask)
        masked = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
        self.sock.sendall(header + masked)

    def _receive(self) -> dict:
        fragments = bytearray()
        while True:
            first, second = self._exact(2)
            opcode, finished = first & 0x0F, bool(first & 0x80)
            length = second & 0x7F
            if length == 126:
                length = struct.unpack("!H", self._exact(2))[0]
            elif length == 127:
                length = struct.unpack("!Q", self._exact(8))[0]
            mask = self._exact(4) if second & 0x80 else None
            payload = self._exact(length)
            if mask:
                payload = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
            if opcode == 9:
                self._send(payload, 10)
                continue
            if opcode in (1, 0):
                fragments.extend(payload)
                if finished:
                    return json.loads(fragments.decode("utf-8"))

    def call(self, method: str, params: dict | None = None) -> dict:
        self.index += 1
        command_id = self.index
        self._send(json.dumps({"id": command_id, "method": method, "params": params or {}}).encode())
        while True:
            message = self._receive()
            if message.get("id") == command_id:
                if "error" in message:
                    raise RuntimeError(f"{method}: {message['error']}")
                return message.get("result", {})


def free_port() -> int:
    with socket.socket() as candidate:
        candidate.bind(("127.0.0.1", 0))
        return candidate.getsockname()[1]


PRODUCT_TEST_CSS = {
    "product": """
        .site-header,.section-heading,.product-panel,.callout-lines,.preview-tools{display:none!important}
        .page-section{min-height:100vh!important;padding:0!important;border:0!important}
        .product-layout{display:block!important;width:100vw!important;height:100vh!important;margin:0!important}
        .product-stage{display:grid!important;width:100%!important;height:100%!important;place-items:center!important}
        .product-view-switch{display:none!important}.product-illustration,#productIllustration{width:min(92vw,1100px)!important;max-height:94vh!important}
    """,
    "scale20": """
        .site-header,.section-heading,.product-panel,.callout-lines,.preview-tools{display:none!important}
        .page-section{min-height:100vh!important;padding:0!important;border:0!important}
        .product-layout{display:block!important;width:100vw!important;height:100vh!important;margin:0!important}
        .product-stage{display:grid!important;width:100%!important;height:100%!important;place-items:center!important}
        .product-view-switch{display:none!important}.product-illustration,#productIllustration{width:20vw!important;max-height:20vh!important}
    """,
    "grayscale": """
        .site-header,.section-heading,.product-panel,.callout-lines,.preview-tools{display:none!important}
        .page-section{min-height:100vh!important;padding:0!important;border:0!important}
        .product-layout{display:block!important;width:100vw!important;height:100vh!important;margin:0!important}
        .product-stage{display:grid!important;width:100%!important;height:100%!important;place-items:center!important;filter:grayscale(1)}
        .product-view-switch{display:none!important}.product-illustration,#productIllustration{width:min(92vw,1100px)!important;max-height:94vh!important;filter:grayscale(1)!important}
    """,
    "blur": """
        .site-header,.section-heading,.product-panel,.callout-lines,.preview-tools{display:none!important}
        .page-section{min-height:100vh!important;padding:0!important;border:0!important}
        .product-layout{display:block!important;width:100vw!important;height:100vh!important;margin:0!important}
        .product-stage{display:grid!important;width:100%!important;height:100%!important;place-items:center!important}
        .product-view-switch{display:none!important}.product-illustration,#productIllustration{width:min(92vw,1100px)!important;max-height:94vh!important;filter:blur(6px)!important}
    """,
    "silhouette": """
        html,body,.product-section{background:#e7e7e2!important}.site-header,.section-heading,.product-panel,.callout-lines,.preview-tools{display:none!important}
        .page-section{min-height:100vh!important;padding:0!important;border:0!important}.product-layout{display:block!important;width:100vw!important;height:100vh!important;margin:0!important}
        .product-stage{display:grid!important;width:100%!important;height:100%!important;place-items:center!important}.product-view-switch{display:none!important}
        .product-illustration,#productIllustration{width:min(92vw,1100px)!important;max-height:94vh!important;filter:brightness(0)!important}.product-illustration *{fill:#111!important;stroke:#111!important;opacity:1!important;filter:none!important}
        .product-illustration .ground-shadow{display:none!important}
    """,
}


def capture(browser: Path, output: Path, width: int, height: int, url: str, profile: Path, delay_ms: int, product_test: str = "") -> None:
    port = free_port()
    process = subprocess.Popen(
        [str(browser), "--headless=new", f"--remote-debugging-port={port}",
         "--remote-allow-origins=*", f"--user-data-dir={profile}", "--disable-gpu-sandbox",
         "--hide-scrollbars", "--no-first-run", "--no-default-browser-check", "about:blank"],
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
        if not targets:
            raise RuntimeError("Chrome DevTools endpoint did not start")
        page = next(target for target in targets if target.get("type") == "page")
        cdp = DevTools(page["webSocketDebuggerUrl"])
        cdp.call("Page.enable")
        cdp.call("Emulation.setDeviceMetricsOverride", {
            "width": width, "height": height, "deviceScaleFactor": 1, "mobile": width <= 430,
            "screenWidth": width, "screenHeight": height,
        })
        cdp.call("Page.navigate", {"url": url})
        cdp.call("Runtime.evaluate", {
            "expression": (
                "new Promise(resolve => {"
                "const started=performance.now();"
                "const ready=()=>{if(document.readyState==='complete'&&document.getElementById('productIllustration'))"
                "setTimeout(resolve," + str(delay_ms) + ");"
                "else if(performance.now()-started>3000)resolve();else requestAnimationFrame(ready)};"
                "if(document.readyState==='complete')ready();else addEventListener('load',ready,{once:true});"
                "})"
            ),
            "awaitPromise": True, "returnByValue": True,
        })
        if product_test:
            css = json.dumps(PRODUCT_TEST_CSS[product_test])
            cdp.call("Runtime.evaluate", {"expression": f"(()=>{{const s=document.createElement('style');s.textContent={css};document.head.appendChild(s);return true}})()", "returnByValue": True})
            time.sleep(.2)
        result = cdp.call("Page.captureScreenshot", {
            "format": "png", "fromSurface": True, "captureBeyondViewport": False,
        })
        output.write_bytes(base64.b64decode(result["data"]))
        cdp.call("Browser.close")
    finally:
        try:
            process.wait(timeout=4)
        except subprocess.TimeoutExpired:
            process.terminate()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--round", default="manual")
    parser.add_argument("--query", default="review=1")
    parser.add_argument("--viewport", help="capture one viewport, for example 1920x1080")
    parser.add_argument("--delay-ms", type=int, default=4500, help="delay after the load event")
    parser.add_argument("--browser", type=Path, help="explicit Chrome/Edge executable")
    parser.add_argument("--product-test", choices=tuple(PRODUCT_TEST_CSS), default="", help="isolate the product for visual quality checks")
    parser.add_argument("--page", type=Path, default=ROOT / "preview.html", help="HTML file to capture")
    args = parser.parse_args()
    browser = args.browser if args.browser and args.browser.exists() else next((candidate for candidate in CHROME if candidate.exists()), None)
    if not browser:
        raise SystemExit("Chrome or Edge was not found")
    output = ROOT / "build" / "ui-review" / args.round
    output.mkdir(parents=True, exist_ok=True)
    page = args.page if args.page.is_absolute() else ROOT / args.page
    url = page.resolve().as_uri() + "?" + args.query
    viewports = VIEWPORTS
    if args.viewport:
        width, height = (int(value) for value in args.viewport.lower().split("x", 1))
        viewports = ((width, height),)
    for width, height in viewports:
        target = output / f"{width}x{height}.png"
        capture(browser, target, width, height, url, output / f"profile-{width}x{height}", max(0, args.delay_ms), args.product_test)
        print(f"captured {width}x{height}: {target}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
