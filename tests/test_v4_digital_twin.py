import re
from pathlib import Path

production=Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview=Path("preview.html").read_text(encoding="utf-8")
model=Path("ui-v4/src/component-model.js").read_text(encoding="utf-8")
twin=Path("ui-v4/src/digital-twin.js").read_text(encoding="utf-8")
ids=re.findall(r'part\("([^"]+)"',model)
assert len(ids)==19 and len(set(ids))==19
for component in ("cup","temp1","temp2","plate","peltier1","peltier2","fan","pico","adc","current1","current2","light","oled","rgb","leds","buttons","power","wifi","webserver"):
    assert component in ids
for group in ("TEMPERATUR","HEIZUNG","BEDIENUNG","SENSOREN","ELEKTRONIK"):
    assert group in twin
for label in ("Temperatursensor 1","Temperatursensor 2","Peltier links","Peltier rechts","Lüfter","OLED","Taster","RGB-Ring","Lichtsensor","Becherschalter","Original-PCB / Pico W","Strommessung"):
    assert label in twin
for behavior in ("toggleConnection","setConnected","setPartState","drawLines","visibleLineIds","renderDetail","setProductView"):
    assert behavior in twin
assert 'live.tone==="configured"' in twin and 'tone:"unmonitored"' in twin
assert 'return id?[id]:[]' in twin
assert 'productView=view==="build"?"build":"exterior"' in twin
assert 'class="component-control"' not in preview and 'class="component-hotspots"' not in preview
assert 'class="callout-lines"' in preview and "data-callout-entry" in twin
assert "callout-detail" in preview and "callout-remove" in preview
assert "component-form" in preview and "part-focused" in twin
assert 'document.body.dataset.cup=' in twin
assert "configuration-drawer" not in preview and "projectHotspots" not in twin and "ProductScene" not in twin
assert "/api/" not in twin
assert "useProfile(profiles" in preview and "useProfile(profiles" not in production
assert "NICHT VERFÜGBAR" in model+twin and "NICHT DIREKT ÜBERWACHT" in model+twin
assert len(production.encode("utf-8")) < 384 * 1024
print(f"callout digital twin checks passed: components={len(ids)}, production={len(production.encode('utf-8'))} bytes")
