from pathlib import Path

production=Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview=Path("preview.html").read_text(encoding="utf-8")
source=Path("ui-v3/src/index.html").read_text(encoding="utf-8")+Path("ui-v3/src/product.svg").read_text(encoding="utf-8")
css=Path("ui-v3/src/experience.css").read_text(encoding="utf-8")+Path("ui-v4/src/digital-twin.css").read_text(encoding="utf-8")
script=Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
for token in ("AUFBAU","REGELKREIS","LIVE","TECHNISCHE DETAILS","PRODUCTILLUSTRATION"):
    assert token in production.upper()
for visual in ("cup-assembly","premium-shell","cutaway-shell","left-jaw","right-jaw","thermal-module","to92","heatsink-part","fan-rotor","pcb-assembly","pico-module","front-console","physical-buttons","rgb-ring"):
    assert visual in source
assert 'data-part="peltier1"' in source and 'data-part="peltier2"' in source
assert 'class="part-placeholder"' in source and "partIn" in css
assert '<svg class="product-illustration"' in source and "<!--__PRODUCT_SVG__-->" in source
assert "contact-face" in source and "button-bank" in source and "product-view-switch" in source
assert source.count('class="fan-blade"')==12
assert 'data-art-variant="a"' in source and 'data-art-variant="b"' in source
assert 'data-art-variant="c"' not in source
assert 'data-product-view="exterior"' in source and 'data-product-view="build"' in source
assert "loop-product" in source and "loop-heater" in source and "loop-sensor" in source
assert 'draw("target","validTarget","#d8ddd6",0,60,[7,6])' in script
assert ".loop-section{min-height:780px" in css and "loop-connector" in source
assert "callout-lines" in css and "mount-silhouette" in css
assert "position:fixed" not in css[css.index(".product-section"):css.index(".loop-section")]
assert 'fetchWithTimeout("/api/unlock"' in production and 'method:"POST"' in production
assert "https://" not in production and "http://" not in production
assert "getContext(\"webgl\"" not in production
print("2.5D SVG product visual contract checks passed")
