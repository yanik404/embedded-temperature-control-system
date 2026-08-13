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
assert preview_ids == production_ids
assert embedded == production
referenced_ids = set(re.findall(r'byId\("([^"]+)"\)', preview))
assert not referenced_ids - preview_ids, f"JavaScript references missing IDs: {sorted(referenced_ids - preview_ids)}"

for element_id in (
    "aufbau", "regelkreis", "live", "technik", "productCanvas", "thermalCanvas",
    "twinHotspots", "componentLens", "configurationDrawer", "simpleLoop",
    "actualTemperature", "timelineCanvas", "controlActions", "unlockDialog",
    "unlockForm", "safetySummary",
):
    assert element_id in production_ids, f"missing interface element: {element_id}"

for token in (
    "/api/status", "/api/unlock", 'command("start")', 'command("stop")',
    "setpoint?value=", 'body:"token="+encodeURIComponent(runtime.token)',
    "Steuerung gesperrt", "Technische Details", "Komponenten anzeigen",
):
    assert token in production, f"missing simple dashboard behavior: {token}"

for scenario in (
    "full-system", "minimal-system", "partial-hardware", "no-sensors", "ready",
    "heating", "holding", "error", "offline", "sensor-error", "fan-error",
    "power-error", "demo30",
):
    assert f'data-scenario="{scenario}"' in preview
assert "data-scenario=" not in production
assert "Obi W-lan Kenobi" not in production

for token in ('getContext("webgl"', 'getContext("2d"', "requestAnimationFrame", "setInterval(poll,POLL_MS)", "Connection: close"):
    assert token in preview + Path("src/webserver.c").read_text(encoding="utf-8")

for token in ("MAX_STEPS 104", "coneY", "cylZ", "torusZ", "partsA", "partsB", "setExploded", "setComponents"):
    assert token in production

assert "onclick=" not in preview
assert "__PREVIEW_MODE__" not in preview + production
assert "/*__CSS__*/" not in preview + production
assert len(production.encode("utf-8")) < 128 * 1024

print(f"dashboard static checks passed: preview={len(preview.encode('utf-8'))} bytes, production={len(production.encode('utf-8'))} bytes, ids={len(production_ids)}")
