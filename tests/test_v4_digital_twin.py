import re
from pathlib import Path


production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview = Path("preview.html").read_text(encoding="utf-8")
model = Path("ui-v4/src/component-model.js").read_text(encoding="utf-8")
twin = Path("ui-v4/src/digital-twin.js").read_text(encoding="utf-8")

ids = re.findall(r'\{id:"([^"]+)"', model)
assert len(ids) == 19 and len(set(ids)) == 19
assert len(re.findall(r'anchor:\[([^\]]+)\]', model)) == 19
for component in ("cup", "temp1", "temp2", "plate", "peltier1", "peltier2", "fan", "pico", "adc", "current1", "current2", "light", "oled", "rgb", "leds", "buttons", "power", "wifi", "webserver"):
    assert component in ids

for token in ("GEPLANT", "ANGESCHLOSSEN", "LIVE", "NICHT DIREKT ÜBERWACHT", "NICHT VERFÜGBAR"):
    assert token in preview + model + twin
assert "setProjectionListener" in twin and "projectHotspots" in twin
assert "toggleConnection" in twin and "setConnected" in twin and "syncModel" in twin
assert "twin-hotspot" in preview and 'class="ths"' in production
assert "component-lens" in preview and 'class="cl"' in production
assert "/api/" not in twin
assert "useProfile(profiles" in preview and "useProfile(profiles" not in production
assert "configuration-drawer" in preview and 'class="cd"' in production
assert len(production.encode("utf-8")) < 128 * 1024

print(f"simple digital twin component checks passed: components={len(ids)}, production={len(production.encode('utf-8'))} bytes")
