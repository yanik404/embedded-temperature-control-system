import hashlib
from pathlib import Path

production=Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview=Path("preview.html").read_text(encoding="utf-8")
index=Path("ui-v3/src/index.html").read_text(encoding="utf-8")
exterior=Path("ui-v7/src/product-v3-exterior.svg").read_text(encoding="utf-8")
cutaway=Path("ui-v5/src/product-v2-cutaway.svg").read_text(encoding="utf-8")
source=index+exterior+cutaway
css=Path("ui-v3/src/experience.css").read_text(encoding="utf-8")+Path("ui-v4/src/digital-twin.css").read_text(encoding="utf-8")
script=Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
for token in ("AUFBAU","REGELKREIS","LIVE","TECHNISCHE DETAILS","PRODUCTILLUSTRATION"):
    assert token in production.upper()
for visual in ("product-v3","product-v3-cup","product-v3-holder","product-v3-base","product-v3-front","product-v3-side-left","product-v3-side-right","product-v3-thermal-covers","product-v3-vent","v2-thermal-interface","v2-peltier-pair","v2-temperature-probes","v2-electronics-deck","v2-cut-fan"):
    assert visual in source
assert 'data-part="peltier1"' in source and 'data-part="peltier2"' in source
assert ".part-placeholder" in css and "partIn" in css
assert '<svg id="productIllustration" class="product-illustration product-v3-illustration"' in exterior
assert "<!--__PRODUCT_V3_EXTERIOR__-->" in source and "<!--__PRODUCT_V2_CUTAWAY__-->" in source
assert 'viewBox="150 76 340 480"' in exterior and 'viewBox="175 72 570 666"' in cutaway
assert "product-view-switch" in source and 'class="fan-blades"' in cutaway
assert 'data-art-variant=' not in exterior+cutaway
assert 'data-part=' not in exterior and 'data-focus=' not in exterior and 'data-callout-anchor=' not in exterior
assert 'data-product-view="exterior"' in source and 'data-product-view="build"' in source
assert 'data-product-illustration="exterior"' in source and 'data-product-illustration="build"' in source
for component in ("temp1","temp2","peltier1","peltier2","fan","oled","buttons","rgb","light","cup","pico","current1"):
    assert f'data-part="{component}"' in source and f'data-callout-anchor="{component}"' in source
assert hashlib.sha256(Path("ui-v3/src/product.svg").read_bytes()).hexdigest().upper()=="16A61414E53C56D95675070C9203A597D02351C56D344B71A5F6AADFBF95BE15"
assert hashlib.sha256(Path("ui-v5/src/product-v2-exterior.svg").read_bytes()).hexdigest().upper()=="D95DC60393548BCE1FFB8D6E75E4A75CBEF73FF1F0D80639759369D5C3E4030A"
assert hashlib.sha256(Path("ui-v5/src/product-v2-cutaway.svg").read_bytes()).hexdigest().upper()=="9B1C81712F5B1B6267EE980BAFF9465FDE8073B0A3E9F30722688A156FC09F31"
assert hashlib.sha256(Path("ui-v6/src/product-v3-rejected-fallback.svg").read_bytes()).hexdigest().upper()=="D70D8F393111E983FDA4C466D962EA73C0DF1C72701509F6AB0102CA21087619"
assert "loop-product" in source and "loop-heater" in source and "loop-sensor" in source
assert 'draw("target","validTarget","#d8ddd6",0,60,[7,6])' in script
assert ".loop-section{min-height:780px" in css and "loop-connector" in source
assert "callout-lines" in css and "mount-silhouette" in css
assert "position:fixed" not in css[css.index(".product-section"):css.index(".loop-section")]
assert 'fetchWithTimeout("/api/unlock"' in production and 'method:"POST"' in production
assert 'href="http' not in production and 'src="http' not in production
assert "getContext(\"webgl\"" not in production
print("independent Product V3 exterior and retained cutaway visual contract checks passed")
