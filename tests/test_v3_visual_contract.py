from pathlib import Path


production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview = Path("preview.html").read_text(encoding="utf-8")
source = Path("ui-v3/src/product-scene.js").read_text(encoding="utf-8")

for token in ("PRODUCTCANVAS", "TIMELINECANVAS", "AUFBAU", "REGELKREIS", "LIVE", "TECHNISCHE DETAILS"):
    assert token in production.upper()
for material in ("pel1", "pel2", "sink", "s1", "s2", "pcb", "console"):
    assert material in source
for behavior in ('classList.add("webgl-ready")', "setTimeout(setup,32)", "setExploded", "setComponents"):
    assert behavior in source

assert "product-fallback" in preview and 'class="pf"' in production
assert "scene-viewport" in preview and 'class="sv"' in production
assert 'fetchWithTimeout("/api/unlock"' in production
assert 'method:"POST"' in production
assert "https://" not in production and "http://" not in production
assert "data-scenario=" not in production

print("simple digital twin visual contract checks passed")
