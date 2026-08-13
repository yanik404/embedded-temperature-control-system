import ast
import re
from html.parser import HTMLParser
from pathlib import Path


class DashboardParser(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.ids = []
        self.external_assets = []

    def handle_starttag(self, tag, attrs):
        values = dict(attrs)
        if "id" in values:
            self.ids.append(values["id"])
        for attribute in ("src", "href"):
            value = values.get(attribute, "")
            if value.startswith(("http://", "https://", "//")):
                self.external_assets.append((tag, attribute, value))


def parse(document):
    parser = DashboardParser()
    parser.feed(document)
    assert len(parser.ids) == len(set(parser.ids)), "dashboard contains duplicate IDs"
    assert not parser.external_assets, f"external assets are not allowed: {parser.external_assets}"
    return set(parser.ids)


preview = Path("preview.html").read_text(encoding="utf-8")
production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
asset_source = Path("include/web_assets.h").read_text(encoding="utf-8")
asset_literals = re.findall(r'^(".*")$', asset_source, flags=re.MULTILINE)
embedded = "".join(ast.literal_eval(literal) for literal in asset_literals)

preview_ids = parse(preview)
production_ids = parse(production)
assert preview_ids == production_ids, "preview and production DOM have diverged"
assert embedded == production, "web_assets.h is not synchronized with production output"

referenced_ids = set(re.findall(r'byId\("([^"]+)"\)', preview))
assert not referenced_ids - preview_ids, f"JavaScript references missing IDs: {sorted(referenced_ids - preview_ids)}"

for element_id in (
    "experience", "productCanvas", "thermalCanvas", "actualTemperature",
    "targetOrbit", "safetyLock", "spatialLoop", "timelineCanvas",
    "engineeringLayer", "piDetails", "presentationButton", "actionControl",
):
    assert element_id in production_ids, f"missing interface element: {element_id}"

assert "/api/status" in production
assert '"/api/"+path' in production
assert 'command("start")' in production
assert 'command("stop")' in production
assert "setpoint?value=" in production

for scenario in (
    "ready", "heating", "holding", "error", "offline", "reconnect",
    "sensor-error", "fan-error", "power-error", "demo30",
):
    assert f'data-scenario="{scenario}"' in preview
assert "data-scenario=" not in production, "simulation controls leaked into Pico build"
assert "Obi W-lan Kenobi" not in production, "simulation data leaked into Pico build"

for token in (
    'getContext("webgl"', 'getContext("2d"', "prefers-reduced-motion",
    "AbortController", "requestAnimationFrame", "setInterval(poll,SAMPLE_INTERVAL)",
    "Connection: close", "START GESPERRT", "u(t) = Kp",
):
    sources = preview + Path("src/webserver.c").read_text(encoding="utf-8")
    assert token in sources, f"missing behavior: {token}"

for token in (
    "MAX_STEPS 88", "explode", "thermal", "fan", "TMP36", "TLA2024",
    "VNH7070", "Istwert-Rückführung", "ENGINEERING", "PRÄSENTATION",
):
    assert token in production, f"missing V3 product experience contract: {token}"

assert "onclick=" not in preview
assert "__PREVIEW_MODE__" not in preview + production
assert "/*__CSS__*/" not in preview + production
assert "<!--__PREVIEW_CONTROLS__-->" not in preview + production
assert len(production.encode("utf-8")) < 128 * 1024, "embedded dashboard exceeded 128 KiB budget"

print(
    "dashboard static checks passed: "
    f"preview={len(preview.encode('utf-8'))} bytes, "
    f"production={len(production.encode('utf-8'))} bytes, "
    f"ids={len(production_ids)}"
)
