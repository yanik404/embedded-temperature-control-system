from pathlib import Path


production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview = Path("preview.html").read_text(encoding="utf-8")
css = Path("ui-v3/src/experience.css").read_text(encoding="utf-8")

assert "productCanvas" in production
assert "scene-viewport" in preview and 'class="sv"' in production
assert "spatial-loop" in production
assert "timeline-layer" in production
assert "card" not in css.lower()

assert "https://" not in production and "http://" not in production
assert "data-scenario=" not in production
assert "data-scenario=\"sensor-error\"" in preview
assert "data-scenario=\"fan-error\"" in preview
assert "data-scenario=\"power-error\"" in preview

for token in ("trace(vec3 ro", "normal(vec3 p)", "fog", "spec", "explode", "coneY", "cylZ", "torusZ"):
    assert token in production, f"missing 3D renderer behavior: {token}"
assert "product-fallback" in preview and 'class="pf"' in production

scene = Path("ui-v3/src/product-scene.js").read_text(encoding="utf-8")
for material in ("pel1", "pel2", "sink", "s1", "s2", "pcb", "console"):
    assert material in scene, f"missing physical model part: {material}"
assert 'classList.add("webgl-ready")' in scene
assert 'setTimeout(setup,32)' in scene
assert 'classList.remove("is-loading")' not in scene

experience = Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
assert "revealWhenReady" in experience
assert "runtime.dataReady&&runtime.visualReady" not in experience  # guarded explicitly for readability/safety
assert 'runtime.dataReady||!runtime.visualReady' in experience

assert 'command("start")' in production
assert "start_allowed" in production
assert "start_block_reason" in production
assert 'method:"POST"' in production

print("V3 visual contract checks passed")
