import re
from pathlib import Path


production = Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
preview = Path("preview.html").read_text(encoding="utf-8")
model = Path("ui-v4/src/component-model.js").read_text(encoding="utf-8")
twin = Path("ui-v4/src/digital-twin.js").read_text(encoding="utf-8")
styles = Path("ui-v4/src/digital-twin.css").read_text(encoding="utf-8")

component_ids = re.findall(r'\{id:"([^"]+)"', model)
assert len(component_ids) == 19, f"expected 19 digital-twin components, found {len(component_ids)}"
assert len(component_ids) == len(set(component_ids)), "duplicate digital-twin component IDs"

for component in (
    "cup", "temp1", "temp2", "plate", "peltier1", "peltier2", "fan", "pico",
    "adc", "current1", "current2", "light", "oled", "rgb", "leds", "buttons",
    "power", "wifi", "webserver",
):
    assert component in component_ids, f"missing component: {component}"

for status in (
    "VORGESEHEN", "ERKANNT", "AKTIV", "MESSUNG GÜLTIG", "NICHT ANGESCHLOSSEN",
    "NICHT ERKANNT", "NICHT VERFÜGBAR", "FEHLER",
):
    assert status in model + twin, f"missing truthful discovery state: {status}"

# A missing optional API field is an explicit third state. It must never be
# coerced to false or presented as a hardware fault.
experience = Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
assert 'fallback===undefined?null:fallback' in experience
assert 'if(supplied===null)return unknown()' in model
assert 'state:"NICHT VERFÜGBAR",tone:"unknown"' in model
assert 'var supplied=s[key];if(supplied===null)return unknown()' in model
assert '!!s[key]' not in model

for mode in ('data-mode="product"', 'data-mode="control"', 'data-mode="thermal"', 'data-mode="engineering"'):
    assert mode in production, f"missing V4 mode: {mode}"

for lens in ('data-lens="product"', 'data-lens="xray"', 'data-lens="signals"'):
    assert lens in production, f"missing digital-twin lens: {lens}"

for story in ("configure", "loop", "control", "analysis", "safety", "engineering"):
    assert f'data-story-step="{story}"' in production, f"missing story chapter: {story}"

for signal in ("temperature", "temperature2", "target", "power", "current1", "current2", "fan", "light"):
    assert f'data-signal="{signal}"' in production, f"missing adaptive analysis signal: {signal}"

for token in (
    "twin-hotspot", "component-lens", "configuration-drawer", "guided-journey",
    "twin-signal-web", "liveFor", "updateSignalAvailability", "startGuide",
    "storyObserver", "setLens", "viewMode",
):
    assert token in production, f"missing V4 behavior: {token}"

assert "Konfiguration ist schreibgeschützt" in production
assert "Änderungen betreffen niemals Hardware" in preview
for scenario in (
    "full-system", "minimal-system", "partial-hardware", "no-sensors",
    "heating", "holding", "sensor-error", "fan-error", "offline",
):
    assert f'data-scenario="{scenario}"' in preview
    assert f'data-scenario="{scenario}"' not in production
assert "setConnected" in preview and "useProfile(profiles" in preview
assert "useProfile(profiles" not in production
assert 'command("start")' in production and "start_allowed" in production
assert "/api/status" in production
assert "/api/" not in twin, "digital-twin preview configuration must not call hardware APIs"
assert "https://" not in production and "http://" not in production
assert "url(http" not in styles.lower()
assert len(production.encode("utf-8")) < 128 * 1024

print(
    "V4 digital twin checks passed: "
    f"components={len(component_ids)}, production={len(production.encode('utf-8'))} bytes"
)
