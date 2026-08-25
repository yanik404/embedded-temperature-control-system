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

# The exterior remains the approved finished-product render. The former
# photorealistic interior stays byte-for-byte available only as a fallback.
assert hashlib.sha256(Path("ui-v7/src/product-finished-exterior-v2.webp").read_bytes()).hexdigest().upper() == "5685C47A2F0F453795E196A39B847B73E919B58C764D93793034C7806B60FE4A"
assert hashlib.sha256(Path("ui-v5/src/product-finished-cutaway-v2.webp").read_bytes()).hexdigest().upper() == "98F0CE756EF9D828815F70EEC1EA20FBCA295D288A7032580A85857D831A1EAA"
assert Path("ui-v7/src/product-finished-exterior-v2.webp").stat().st_size == 81046
assert Path("ui-v5/src/product-finished-cutaway-v2.webp").stat().st_size == 90748

# Exact delivered STEP artwork stays byte-for-byte available as fallback.
assert hashlib.sha256(Path("ui-v7/src/product-step-exterior.webp").read_bytes()).hexdigest().upper() == "41036BD0F2D5ED0AD315EB8C17F1E5E5E4E5DFA20861DEEB4AA8DB8E813DAD66"
assert hashlib.sha256(Path("ui-v5/src/product-step-cutaway.webp").read_bytes()).hexdigest().upper() == "3C15E2C3C1E2042746ACEA181CE4076A9C9F7247115B2E2CAAF1C5D83A9A3B5B"
assert hashlib.sha256(Path("ui-v7/src/product-finished-exterior.webp").read_bytes()).hexdigest().upper() == "443E385CACAD073A0EA656E49B6C93B2BD8AFB3F8476095DBF2979BB267AE926"
assert hashlib.sha256(Path("ui-v5/src/product-finished-cutaway.webp").read_bytes()).hexdigest().upper() == "A408487E05EBA5D3ACA1D9104DFEFEA7904F983F96B92394C99153A5D23AA408"

assert "__PRODUCT_EXTERIOR__" in exterior and "__PRODUCT_CUTAWAY__" not in cutaway
assert "__PCB_ORIGINAL__" not in cutaway
assert exterior.count("<image ") == 1 and cutaway.count("<image ") == 0
assert production.count("data:image/webp;base64,") == 1
assert 'viewBox="0 0 955 1100"' in exterior and 'viewBox="0 0 702 1100"' in cutaway
assert '<svg class="product-illustration product-v3-illustration product-step-render" id="productIllustration"' in exterior
assert 'id="product-v3"' in exterior
assert 'class="product-v3-cup-state"' in exterior
assert 'class="product-v3-liquid-tint"' in exterior
assert 'class="product-v3-empty-holder"' in exterior
assert 'class="cutaway-cup-state-tint"' in cutaway
assert 'class="cutaway-live-cup"' in cutaway
for state in ('data-state="AUFHEIZEN"', 'data-state="HALTEN"', 'data-state="FEHLER"'):
    assert state in css
assert "--cup-state-color" in css and "blue means cold/ready" in css
assert 'data-cup="present"' in css and "S_DETECT value controls cup presence" in css
assert "document.body.dataset.cup" in script
assert "data-part=" not in exterior and "data-focus=" not in exterior and "data-callout-anchor=" not in exterior

# The rebuilt interior is intentionally a compact, self-contained vector. Each
# real component remains focusable once and modes reuse vector definitions.
for component in ("temp1", "temp2", "peltier1", "peltier2", "fan", "oled", "buttons", "rgb", "light", "cup", "pico", "current1"):
    assert cutaway.count(f'data-part="{component}"') == 1
    assert cutaway.count(f'data-focus="{component}"') == 1
    assert cutaway.count(f'data-callout-anchor="{component}"') == 1
assert "component-real" not in cutaway and "cup-detector-plunger" not in cutaway
for component in ("peltier1", "peltier2", "temp1", "temp2", "fan", "cup", "pico", "oled", "buttons"):
    assert f'class="component-part" data-part="{component}" data-focus="{component}"' in cutaway
for token in ("simple-cutaway", "simple-pcb", "simple-switch", "simple-fan", "PCB + PICO W", "DETECT"):
    assert token in cutaway
# The simplified stack keeps the physical order readable: compact side heaters,
# horizontal fan below the cup, PCB plane, then centred front controls.
for placement in (
    'x="143" y="326" width="76" height="166"',
    'transform="translate(351 620) scale(1 .38)"',
    "LÜFTER · HORIZONTAL",
    'cx="351" cy="832" rx="205" ry="70"',
    "M172 817a17 17",
    'x="273" y="932" width="156" height="78"',
    '<circle cx="302" cy="1052" r="11"',
):
    assert placement in cutaway
for forbidden in ("cutawayRaster", "sinkFins", "fan-blades", "KÜHLKÖRPER", "HEATSINK", "thermal-left", "thermal-right", "pcb-module"):
    assert forbidden not in cutaway
assert "step-hotspot" in cutaway and ".product-step-render .step-hotspot" in css
assert "<image " not in cutaway and "__PRODUCT_CUTAWAY__" not in production
for mode in ("thermal", "sensors", "electronics"):
    assert f'data-mode-slice="{mode}"' in cutaway
    assert f'data-mode-marker="{mode}"' in cutaway

assert "product-view-switch" in source
assert "<!--__PRODUCT_V3_EXTERIOR__-->" in source and "<!--__PRODUCT_V2_CUTAWAY__-->" in source
assert 'data-product-view="exterior"' in source and 'data-product-view="build"' in source
assert 'data-product-illustration="exterior"' in source and 'data-product-illustration="build"' in source
for mode in ("all", "thermal", "sensors", "electronics"):
    assert f'data-build-mode="{mode}"' in source
assert "partFocusCard" in source

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
