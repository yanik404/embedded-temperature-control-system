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
    "thermalHero", "assemblyStage", "setpointDial", "safetyInterlock",
    "controlLoop", "loopVisual", "signalHistory", "historyCanvas",
    "hardwareMap", "eventTimeline", "engineeringDrawer", "presentationToggle",
):
    assert element_id in production_ids, f"missing interface element: {element_id}"

assert "/api/status" in production
assert "`/api/${path}`" in production
assert 'command("start")' in production
assert 'command("stop")' in production
assert "setpoint?value=" in production

for scenario in ("ready", "heating", "holding", "error", "disconnected", "recovery", "demo30"):
    assert f'data-scenario="{scenario}"' in preview
assert "data-scenario=" not in production, "simulation controls leaked into Pico build"
assert "const nominal" not in production, "simulation data leaked into Pico build"

for token in (
    "getContext(\"webgl\"", "getContext(\"2d\"", "prefers-reduced-motion",
    "AbortController", "requestAnimationFrame", "setInterval(poll, SAMPLE_INTERVAL)",
    "Connection: close", "SAFETY LOCK", "Kp · e + Ki",
):
    assert token in preview or token in Path("src/webserver.c").read_text(encoding="utf-8"), f"missing behavior: {token}"

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
