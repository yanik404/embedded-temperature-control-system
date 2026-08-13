import hashlib
from pathlib import Path

production=Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview=Path("preview.html").read_text(encoding="utf-8")
index=Path("ui-v3/src/index.html").read_text(encoding="utf-8")
exterior=Path("ui-v5/src/product-v2-exterior.svg").read_text(encoding="utf-8")
cutaway=Path("ui-v5/src/product-v2-cutaway.svg").read_text(encoding="utf-8")
source=index+exterior+cutaway
css=Path("ui-v3/src/experience.css").read_text(encoding="utf-8")+Path("ui-v4/src/digital-twin.css").read_text(encoding="utf-8")
script=Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
for token in ("AUFBAU","REGELKREIS","LIVE","TECHNISCHE DETAILS","PRODUCTILLUSTRATION"):
    assert token in production.upper()
for visual in ("v2-cup","v2-housing","v2-side-modules","v2-front-panel","v2-side-vent","v2-thermal-interface","v2-peltier-pair","v2-temperature-probes","v2-electronics-deck","v2-cut-fan"):
    assert visual in source
assert 'data-part="peltier1"' in source and 'data-part="peltier2"' in source
assert 'class="part-placeholder"' in source and "partIn" in css
assert '<svg class="product-illustration product-v2-illustration"' in source
assert "<!--__PRODUCT_V2_EXTERIOR__-->" in source and "<!--__PRODUCT_V2_CUTAWAY__-->" in source
assert 'viewBox="175 72 570 666"' in exterior and 'viewBox="175 72 570 666"' in cutaway
assert "product-view-switch" in source and 'class="fan-blades"' in cutaway
assert 'data-art-variant=' not in exterior+cutaway
assert 'data-product-view="exterior"' in source and 'data-product-view="build"' in source
assert 'data-product-illustration="exterior"' in source and 'data-product-illustration="build"' in source
for component in ("temp1","temp2","peltier1","peltier2","fan","oled","buttons","rgb","light","cup","pico","current1"):
    assert f'data-part="{component}"' in source and f'data-callout-anchor="{component}"' in source
assert hashlib.sha256(Path("ui-v3/src/product.svg").read_bytes()).hexdigest().upper()=="16A61414E53C56D95675070C9203A597D02351C56D344B71A5F6AADFBF95BE15"
assert "loop-product" in source and "loop-heater" in source and "loop-sensor" in source
assert 'draw("target","validTarget","#d8ddd6",0,60,[7,6])' in script
assert ".loop-section{min-height:780px" in css and "loop-connector" in source
assert "callout-lines" in css and "mount-silhouette" in css
assert "position:fixed" not in css[css.index(".product-section"):css.index(".loop-section")]
assert 'fetchWithTimeout("/api/unlock"' in production and 'method:"POST"' in production
assert 'href="http' not in production and 'src="http' not in production
assert "getContext(\"webgl\"" not in production
print("independent Product V2 exterior/cutaway visual contract checks passed")
