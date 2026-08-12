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


preview = Path("preview.html").read_text(encoding="utf-8")
asset_source = Path("include/web_assets.h").read_text(encoding="utf-8")
asset_literals = re.findall(r'^"(.*)"$', asset_source, flags=re.MULTILINE)
embedded = "".join(ast.literal_eval(f'"{literal}"') for literal in asset_literals)

parser = DashboardParser()
parser.feed(preview)

assert len(parser.ids) == len(set(parser.ids)), "dashboard contains duplicate element IDs"
assert not parser.external_assets, f"external assets are not allowed: {parser.external_assets}"

referenced_ids = set(re.findall(r'byId\("([^"]+)"\)', preview))
missing_ids = sorted(referenced_ids - set(parser.ids))
assert not missing_ids, f"JavaScript references missing DOM IDs: {missing_ids}"

for endpoint in ("/api/status", "/api/setpoint"):
    assert endpoint in preview, f"missing API endpoint: {endpoint}"
assert "`/api/${path}`" in preview
assert 'command("start")' in preview
assert 'command("stop")' in preview

for section_id in (
    "overview",
    "control-loop",
    "analytics",
    "hardware",
    "details",
    "eventList",
    "presentationToggle",
):
    assert section_id in parser.ids, f"missing dashboard section: {section_id}"

for scenario in ("ready", "heating", "holding", "error", "disconnected"):
    assert f'data-scenario="{scenario}"' in preview

assert '<meta name="viewport"' in preview
assert "prefers-reduced-motion" in preview
assert "AbortController" in preview
assert "setInterval(poll,500)" in preview
assert embedded == preview, "include/web_assets.h is not synchronized with preview.html"

style = re.search(r"<style>(.*?)</style>", preview, flags=re.DOTALL).group(1)
script = re.search(r"<script>(.*?)</script>", preview, flags=re.DOTALL).group(1)
print(
    "dashboard static checks passed: "
    f"html={len(preview.encode('utf-8'))} bytes, "
    f"css={len(style.encode('utf-8'))} bytes, "
    f"javascript={len(script.encode('utf-8'))} bytes, "
    f"ids={len(parser.ids)}"
)
