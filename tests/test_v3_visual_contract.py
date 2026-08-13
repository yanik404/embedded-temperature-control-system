from pathlib import Path

production=Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview=Path("preview.html").read_text(encoding="utf-8")
source=Path("ui-v3/src/index.html").read_text(encoding="utf-8")
css=Path("ui-v3/src/experience.css").read_text(encoding="utf-8")
for token in ("AUFBAU","REGELKREIS","LIVE","TECHNISCHE DETAILS","PRODUCTILLUSTRATION"):
    assert token in production.upper()
for visual in ("cup-assembly","holder-assembly","left-jaw","right-jaw","to92","fan-housing","fan-rotor","pcb-assembly","pico-module","adc-chip","front-console","physical-buttons","rgb-ring","power-jack"):
    assert visual in source
assert 'data-part="peltier1"' in source and 'data-part="peltier2"' in source
assert 'class="part-placeholder"' in source and "partIn" in css
assert "loop-section{min-height:850px;background:#131512}" in css
assert "position:fixed" not in css[css.index(".product-section"):css.index(".loop-section")]
assert 'fetchWithTimeout("/api/unlock"' in production and 'method:"POST"' in production
assert "https://" not in production and "http://" not in production
assert "getContext(\"webgl\"" not in production
print("2.5D SVG product visual contract checks passed")
