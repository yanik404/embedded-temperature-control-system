import hashlib
from pathlib import Path


production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview = Path("preview.html").read_text(encoding="utf-8")
index = Path("ui-v3/src/index.html").read_text(encoding="utf-8")
exterior = Path("ui-v7/src/product-v3-exterior.svg").read_text(encoding="utf-8")
cutaway = Path("ui-v5/src/product-v2-cutaway.svg").read_text(encoding="utf-8")
css = Path("ui-v3/src/experience.css").read_text(encoding="utf-8") + Path("ui-v4/src/digital-twin.css").read_text(encoding="utf-8")
script = Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
source = index + exterior + cutaway

for token in ("AUFBAU", "REGELKREIS", "LIVE", "TECHNISCHE DETAILS", "PRODUCTILLUSTRATION"):
    assert token in production.upper()

# The active transparent WebP files are finished-product renders derived from
# the supplied STEP references and remain compact enough for Pico flash.
assert hashlib.sha256(Path("ui-v7/src/product-finished-exterior.webp").read_bytes()).hexdigest().upper() == "443E385CACAD073A0EA656E49B6C93B2BD8AFB3F8476095DBF2979BB267AE926"
assert hashlib.sha256(Path("ui-v5/src/product-finished-cutaway.webp").read_bytes()).hexdigest().upper() == "A408487E05EBA5D3ACA1D9104DFEFEA7904F983F96B92394C99153A5D23AA408"
assert Path("ui-v7/src/product-finished-exterior.webp").stat().st_size == 74560
assert Path("ui-v5/src/product-finished-cutaway.webp").stat().st_size == 107528

# Exact delivered STEP artwork stays byte-for-byte available as fallback.
assert hashlib.sha256(Path("ui-v7/src/product-step-exterior.webp").read_bytes()).hexdigest().upper() == "41036BD0F2D5ED0AD315EB8C17F1E5E5E4E5DFA20861DEEB4AA8DB8E813DAD66"
assert hashlib.sha256(Path("ui-v5/src/product-step-cutaway.webp").read_bytes()).hexdigest().upper() == "3C15E2C3C1E2042746ACEA181CE4076A9C9F7247115B2E2CAAF1C5D83A9A3B5B"

assert "__PRODUCT_EXTERIOR__" in exterior and "__PRODUCT_CUTAWAY__" in cutaway
assert exterior.count("<image ") == 1 and cutaway.count("<image ") == 1
assert production.count("data:image/webp;base64,") == 2
assert 'viewBox="0 0 1070 1100"' in exterior and 'viewBox="0 0 999 1100"' in cutaway
assert '<svg class="product-illustration product-v3-illustration product-step-render" id="productIllustration"' in exterior
assert 'id="product-v3"' in exterior
assert "data-part=" not in exterior and "data-focus=" not in exterior and "data-callout-anchor=" not in exterior

# Only transparent interaction geometry may sit above the supplied cutaway.
for component in ("temp1", "temp2", "peltier1", "peltier2", "fan", "oled", "buttons", "rgb", "light", "cup", "pico", "current1"):
    assert cutaway.count(f'data-part="{component}"') == 1
    assert cutaway.count(f'data-callout-anchor="{component}"') == 1
assert cutaway.count('data-part="fan"') == 1
for forbidden in ("sinkFins", "fan-blades", "KÜHLKÖRPER", "HEATSINK", "thermal-left", "thermal-right", "pcb-module"):
    assert forbidden not in cutaway
assert "step-hotspot" in cutaway and ".product-step-render .step-hotspot" in css

assert "product-view-switch" in source
assert "<!--__PRODUCT_V3_EXTERIOR__-->" in source and "<!--__PRODUCT_V2_CUTAWAY__-->" in source
assert 'data-product-view="exterior"' in source and 'data-product-view="build"' in source
assert 'data-product-illustration="exterior"' in source and 'data-product-illustration="build"' in source

# Legacy art remains available as an untouched fallback, but is not embedded.
assert hashlib.sha256(Path("ui-v3/src/product.svg").read_bytes()).hexdigest().upper() == "16A61414E53C56D95675070C9203A597D02351C56D344B71A5F6AADFBF95BE15"
assert hashlib.sha256(Path("ui-v5/src/product-v2-exterior.svg").read_bytes()).hexdigest().upper() == "D95DC60393548BCE1FFB8D6E75E4A75CBEF73FF1F0D80639759369D5C3E4030A"
assert hashlib.sha256(Path("ui-v6/src/product-v3-rejected-fallback.svg").read_bytes()).hexdigest().upper() == "D70D8F393111E983FDA4C466D962EA73C0DF1C72701509F6AB0102CA21087619"

assert "loop-product" in source and "loop-heater" in source and "loop-sensor" in source
assert 'draw("target","validTarget","#d8ddd6",0,60,[7,6])' in script
assert ".loop-section{min-height:780px" in css and "loop-connector" in source
assert "callout-lines" in css
assert "position:fixed" not in css[css.index(".product-section"):css.index(".loop-section")]
assert 'fetchWithTimeout("/api/unlock"' in production and 'method:"POST"' in production
assert 'href="http' not in production and 'src="http' not in production
assert "getContext(\"webgl\"" not in production

print("finished-product exterior/cutaway asset and interaction contract checks passed")
