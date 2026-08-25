#!/usr/bin/env python3
"""Build the readable local preview and the single-file Pico dashboard asset."""

from __future__ import annotations

import argparse
import base64
import gzip
import json
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "ui-v3" / "src"
V4_SOURCE = ROOT / "ui-v4" / "src"
V5_SOURCE = ROOT / "ui-v5" / "src"
V7_SOURCE = ROOT / "ui-v7" / "src"
PREVIEW = ROOT / "preview.html"
PRODUCTION = ROOT / "ui" / "dist" / "dashboard.production.html"
HEADER = ROOT / "include" / "web_assets.h"
STATS = ROOT / "ui" / "dist" / "build-stats.json"

PREVIEW_CONTROLS = """
<div class="preview-tools">
 <button class="preview-tools-toggle" type="button" aria-expanded="false">DEMO TOOLS</button>
 <div class="preview-deck" aria-label="Lokale Vorschau-Szenarien" hidden>
 <div class="preview-view-switch" role="group" aria-label="Vorschaugröße">
  <strong>ANSICHT</strong>
  <button type="button" data-preview-viewport="desktop">PC · 1440 × 900</button>
  <button type="button" data-preview-viewport="mobile">HANDY · 390 × 844</button>
 </div>
 <button type="button" data-scenario="full-system">FULL SYSTEM</button>
 <button type="button" data-scenario="minimal-system">MINIMAL SYSTEM</button>
 <button type="button" data-scenario="partial-hardware">PARTIAL HARDWARE</button>
 <button type="button" data-scenario="no-sensors">NO SENSORS</button>
 <button type="button" data-scenario="ready">READY</button>
 <button type="button" data-scenario="heating" class="active">HEATING</button>
 <button type="button" data-scenario="cooling">COOLING</button>
 <button type="button" data-scenario="holding">HOLDING</button>
 <button type="button" data-scenario="error">ERROR</button>
 <button type="button" data-scenario="disconnect">DISCONNECT</button>
 <button type="button" data-scenario="recovery">RECOVERY</button>
 <button type="button" data-scenario="sensor-error">SENSOR ERROR</button>
 <button type="button" data-scenario="fan-error">FAN ERROR</button>
 <button type="button" data-scenario="power-error">POWER ERROR</button>
  <button type="button" data-scenario="demo30">30 MIN DEMO</button>
 </div>
</div>
<dialog class="preview-viewport-dialog" aria-label="Lokale responsive Vorschau">
 <header><div><small>LOKALE RESPONSIVE VORSCHAU</small><strong class="preview-viewport-title">PC-ANSICHT · 1440 × 900</strong></div><nav class="preview-dialog-actions" aria-label="Vorschauansicht wechseln"><button type="button" data-preview-viewport="desktop">PC</button><button type="button" data-preview-viewport="mobile">HANDY</button><button class="preview-viewport-close" type="button" aria-label="Vorschau schließen">×</button></nav></header>
 <div class="preview-viewport-stage"><div class="preview-device-frame"><iframe class="preview-viewport-frame" title="Responsive Dashboard-Vorschau"></iframe></div></div>
</dialog>
<style>
.preview-view-switch{display:grid;grid-template-columns:1fr 1fr;gap:4px;padding:4px 0 7px;border-bottom:1px solid #394039}.preview-view-switch strong{grid-column:1/-1;padding:3px;color:#747d75;font-size:8px;letter-spacing:.12em}.preview-view-switch button{border:1px solid #394039!important;border-radius:3px;text-align:center!important}.preview-view-switch button.active{color:#fff;background:#243229}.preview-frame .preview-tools{display:none!important}.preview-viewport-dialog{width:min(1500px,calc(100vw - 20px));height:min(960px,calc(100vh - 20px));max-width:none;max-height:none;margin:auto;padding:0;border:1px solid #3a423c;border-radius:10px;background:#0c0f0d;color:#f2f1ea;overflow:hidden}.preview-viewport-dialog::backdrop{background:#050605e8;backdrop-filter:blur(7px)}.preview-viewport-dialog>header{display:flex;height:58px;align-items:center;justify-content:space-between;padding:0 18px;border-bottom:1px solid #303631;background:#121512}.preview-viewport-dialog>header small,.preview-viewport-dialog>header strong{display:block}.preview-viewport-dialog>header small{color:#7f8881;font-size:8px;letter-spacing:.12em}.preview-viewport-dialog>header strong{margin-top:2px;font-size:13px}.preview-dialog-actions{display:flex;gap:5px}.preview-dialog-actions button{min-width:48px;height:34px;padding:0 10px;border:1px solid #3a423c;border-radius:5px;background:transparent;color:#aeb5af;font-size:9px;cursor:pointer}.preview-dialog-actions button.active{border-color:#667269;color:#fff;background:#263029}.preview-dialog-actions .preview-viewport-close{min-width:36px;padding:0;color:#dfe3dc;font-size:24px}.preview-viewport-stage{height:calc(100% - 58px);padding:16px;background:#080a08;overflow:auto}.preview-device-frame{position:relative;margin:0 auto;background:#111310;box-shadow:0 20px 70px #000b}.preview-device-frame.mobile{border:7px solid #292e2a;border-radius:28px;box-shadow:0 24px 80px #000}.preview-device-frame iframe{position:absolute;top:0;left:0;border:0;background:#111310;transform-origin:top left}.preview-device-frame.mobile iframe{border-radius:20px}
</style>
""".strip()


def read(name: str) -> str:
    return (SOURCE / name).read_text(encoding="utf-8")


def read_v4(name: str) -> str:
    return (V4_SOURCE / name).read_text(encoding="utf-8")


def read_v5(name: str) -> str:
    return (V5_SOURCE / name).read_text(encoding="utf-8")


def read_v7(name: str) -> str:
    return (V7_SOURCE / name).read_text(encoding="utf-8")


def image_data(source: Path, mime: str = "image/webp") -> str:
    return f"data:{mime};base64," + base64.b64encode(source.read_bytes()).decode("ascii")


def minify_css(source: str) -> str:
    source = re.sub(r"/\*.*?\*/", "", source, flags=re.DOTALL)
    source = re.sub(r"\s+", " ", source)
    source = re.sub(r"\s*([{}:;,>])\s*", r"\1", source)
    source = re.sub(r";}", "}", source)
    return source.strip()


def shorten_css_selectors(css: str, html: str, javascript: str) -> tuple[str, str, str]:
    replacements = {
        "section-heading": "sh", "callout-entry": "ce", "component-legend": "cl",
        "product-section": "ps", "product-stage": "pg", "timeline-layer": "tl",
        "technical-grid": "tg", "simple-loop": "sl", "loop-step": "ls",
    }
    for source, target in replacements.items():
        css = css.replace(source, target)
        html = html.replace(source, target)
        javascript = javascript.replace(source, target)
    return css, html, javascript


def minify_javascript(source: str) -> str:
    """Remove lexical whitespace while preserving strings and the GLSL template."""
    def compact_glsl(match: re.Match[str]) -> str:
        directives: list[str] = []
        lines: list[str] = []
        for line in match.group(1).splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                token = f"GLSLDIRECTIVE{len(directives)}X"
                directives.append(stripped)
                lines.append(token)
            else:
                lines.append(stripped)
        body = re.sub(r"\s+", " ", " ".join(lines)).strip()
        body = re.sub(r"\s*([{}();,\[\]+*/<>=?:-])\s*", r"\1", body)
        for index, directive in enumerate(directives):
            body = body.replace(f"GLSLDIRECTIVE{index}X", "\n" + directive + "\n")
        return "var fragment=`" + body + "`"

    source = re.sub(r"var fragment=`(.*?)`", compact_glsl, source, flags=re.DOTALL)
    output: list[str] = []
    index = 0
    pending_space = False
    quote = ""
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if quote:
            output.append(char)
            if char == "\\" and following:
                output.append(following)
                index += 2
                continue
            if char == quote:
                quote = ""
            index += 1
            continue
        if char in ('"', "'", "`"):
            if pending_space and output and (output[-1][-1].isalnum() or output[-1][-1] in "_$"):
                output.append(" ")
            pending_space = False
            quote = char
            output.append(char)
            index += 1
            continue
        if char == "/" and following == "*":
            end = source.find("*/", index + 2)
            index = len(source) if end < 0 else end + 2
            pending_space = True
            continue
        if char == "/" and following == "/":
            end = source.find("\n", index + 2)
            index = len(source) if end < 0 else end + 1
            pending_space = True
            continue
        if char.isspace():
            pending_space = True
            index += 1
            continue
        if pending_space and output:
            previous = output[-1][-1]
            if (previous.isalnum() or previous in "_$") and (char.isalnum() or char in "_$"):
                output.append(" ")
            elif previous in "+-" and char == previous:
                output.append(" ")
        pending_space = False
        output.append(char)
        index += 1
    return "".join(output)


def compose(*, preview: bool, minified: bool) -> tuple[str, str, str]:
    template = read("index.html")
    exterior = read_v7("product-v3-exterior.svg").replace("__PRODUCT_EXTERIOR__", image_data(V7_SOURCE / "product-finished-exterior-v2.webp"))
    cutaway = read_v5("product-v2-cutaway.svg")
    template = template.replace("<!--__PRODUCT_V3_EXTERIOR__-->", exterior)
    template = template.replace("<!--__PRODUCT_V2_CUTAWAY__-->", cutaway)
    css = "\n\n".join([read("experience.css"), read_v4("digital-twin.css")])
    scripts = [read_v4("component-model.js"), read_v4("digital-twin.js")]
    if preview:
        scripts.append(read("preview.js"))
    scripts.append(read("experience.js").replace("__PREVIEW_MODE__", "true" if preview else "false"))
    javascript = "\n\n".join(scripts)
    if minified:
        css = minify_css(css)
        javascript = minify_javascript(javascript)
        css, template, javascript = shorten_css_selectors(css, template, javascript)
    html = template.replace("/*__CSS__*/", css)
    html = html.replace("<!--__PREVIEW_CONTROLS__-->", PREVIEW_CONTROLS if preview else "")
    html = html.replace("/*__SCRIPT__*/", javascript)
    if minified:
        html = re.sub(r">\s+<", "><", html)
        html = re.sub(r"\s+([{}])", r"\1", html)
        html = re.sub(r"\s+(aria-hidden|aria-label|aria-labelledby|aria-live|aria-pressed|aria-expanded|aria-controls|role)=\"[^\"]*\"", "", html)
        html = re.sub(r"\s+class=\"([^\"]+)\"", lambda match: f' class="{match.group(1)}"', html)
        html = re.sub(r"[ \t]+\n", "\n", html)
        html = html.strip() + "\n"
    return html, css, javascript


def c_header(html: str) -> str:
    fragments = html.splitlines(keepends=True)
    literals = "\n".join(json.dumps(fragment, ensure_ascii=False) for fragment in fragments)
    return (
        "#pragma once\n\n"
        "/* Generated by tools/build_ui.py. Edit files under ui-v3/src, ui-v4/src, ui-v5/src and ui-v7/src. */\n"
        "static const char dashboard_html[] =\n"
        f"{literals}\n;\n"
    )


def build_stats(preview_html: str, production_html: str, css: str, javascript: str) -> dict:
    size = lambda value: len(value.encode("utf-8"))
    compressed = lambda value: len(gzip.compress(value.encode("utf-8"), compresslevel=9, mtime=0))
    return {
        "preview_bytes": size(preview_html),
        "production_bytes": size(production_html),
        "production_gzip_bytes_theoretical": compressed(production_html),
        "production_css_bytes": size(css),
        "production_javascript_bytes": size(javascript),
        "external_library_bytes": 0,
        "runtime_http_requests_for_assets": 0,
        "webgl_draw_calls_per_frame": 0,
        "product_rendering": "photoreal_exterior_with_simple_vector_cutaway",
        "history_max_seconds": 1800,
    }


def write_or_check(path: Path, content: str, check: bool) -> bool:
    existing = path.read_text(encoding="utf-8") if path.exists() else None
    if existing == content:
        return True
    if check:
        print(f"out of date: {path.relative_to(ROOT)}", file=sys.stderr)
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8", newline="\n")
    print(f"wrote {path.relative_to(ROOT)}")
    return True


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="fail if generated assets differ")
    args = parser.parse_args()
    preview_html, _, _ = compose(preview=True, minified=False)
    production_html, production_css, production_js = compose(preview=False, minified=True)
    stats = build_stats(preview_html, production_html, production_css, production_js)
    outputs = {
        PREVIEW: preview_html,
        PRODUCTION: production_html,
        HEADER: c_header(production_html),
        STATS: json.dumps(stats, indent=2, sort_keys=True) + "\n",
    }
    current = all(write_or_check(path, content, args.check) for path, content in outputs.items())
    print(
        "UI build: "
        f"preview={stats['preview_bytes']} B, "
        f"production={stats['production_bytes']} B, "
        f"gzip~{stats['production_gzip_bytes_theoretical']} B, "
        f"libraries={stats['external_library_bytes']} B"
    )
    return 0 if current else 1


if __name__ == "__main__":
    raise SystemExit(main())
