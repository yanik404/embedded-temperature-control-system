from pathlib import Path


production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview = Path("preview.html").read_text(encoding="utf-8")
css = Path("ui-v3/src/experience.css").read_text(encoding="utf-8")

assert "productCanvas" in production
assert "scene-viewport" in production
assert "spatial-loop" in production
assert "timeline-layer" in production
assert "card" not in css.lower()

assert "https://" not in production and "http://" not in production
assert "data-scenario=" not in production
assert "data-scenario=\"sensor-error\"" in preview
assert "data-scenario=\"fan-error\"" in preview
assert "data-scenario=\"power-error\"" in preview

for token in ("trace(vec3 ro", "normal(vec3 p)", "fog", "spec", "explode", "product-fallback"):
    assert token in production, f"missing 3D renderer behavior: {token}"

assert 'command("start")' in production
assert "start_allowed" in production
assert "start_block_reason" in production
assert 'method:"POST"' in production

print("V3 visual contract checks passed")
