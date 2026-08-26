import ast
import re
from html.parser import HTMLParser
from pathlib import Path

class DashboardParser(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True); self.ids=[]; self.external_assets=[]
    def handle_starttag(self,tag,attrs):
        values=dict(attrs)
        if "id" in values:self.ids.append(values["id"])
        for attribute in ("src","href"):
            value=values.get(attribute,"")
            if value.startswith(("http://","https://","//")):self.external_assets.append((tag,attribute,value))

def parse(document):
    parser=DashboardParser(); parser.feed(document)
    assert len(parser.ids)==len(set(parser.ids)),"dashboard contains duplicate IDs"
    assert not parser.external_assets,f"external assets are not allowed: {parser.external_assets}"
    return set(parser.ids)

preview=Path("preview.html").read_text(encoding="utf-8")
production=Path("ui/dist/dashboard.production.html").read_text(encoding="utf-8")
asset_source=Path("include/web_assets.h").read_text(encoding="utf-8")
embedded="".join(ast.literal_eval(item) for item in re.findall(r'^(".*")$',asset_source,flags=re.MULTILINE))
preview_ids=parse(preview); production_ids=parse(production)
assert preview_ids==production_ids
assert embedded==production
referenced_ids=set(re.findall(r'byId\("([^"]+)"\)',preview))
assert not referenced_ids-preview_ids,f"JavaScript references missing IDs: {sorted(referenced_ids-preview_ids)}"
for element_id in ("aufbau","regelkreis","live","technik","productIllustration","calloutLines","calloutPaths","configurationList","componentCount","ambientMode","ambientSource","cupPresence","cupPresenceTitle","cupPresenceSignal","simpleLoop","actualTemperature","timelineCanvas","controlActions","unlockDialog","unlockForm","safetySummary","stopButton"):
    assert element_id in production_ids,f"missing interface element: {element_id}"
for token in ("/api/status","/api/unlock",'command("start",true)','command("stop",false)',"setpoint?value=",'body:token?"token="+encodeURIComponent(token):""',"Steuerung gesperrt","Technische Details","Mehr Messwerte"):
    assert token in production,f"missing dashboard behavior: {token}"
for scenario in ("full-system","minimal-system","partial-hardware","no-sensors","ready","day-mode","night-mode","heating","cooling","holding","error","disconnect","recovery","sensor-error","fan-error","power-error","demo30"):
    assert f'data-scenario="{scenario}"' in preview
assert "data-scenario=" not in production
assert '<div class="preview-tools">' in preview and '<div class="preview-tools">' not in production
assert 'class="preview-deck" aria-label="Lokale Vorschau-Szenarien" hidden' in preview
assert 'data-preview-viewport="desktop"' in preview and 'data-preview-viewport="mobile"' in preview
assert 'data-preview-viewport=' not in production
assert 'class="preview-viewport-dialog"' in preview and 'class="preview-viewport-dialog"' not in production
assert 'data-theme-choice="auto"' in production and 'data-theme-choice="day"' in production and 'data-theme-choice="night"' in production
assert 'data-theme="night" data-theme-mode="auto"' in production
assert "Obi W-lan Kenobi" not in production
assert 'getContext("2d"' in production and 'getContext("webgl"' not in preview+production
assert "productCanvas" not in preview+production and "thermalCanvas" not in preview+production
assert "requestAnimationFrame" in preview and "setInterval(poll,POLL_MS)" in production
assert "onclick=" not in preview and "__PREVIEW_MODE__" not in preview+production
assert len(production.encode("utf-8")) < 384 * 1024
print(f"dashboard static checks passed: preview={len(preview.encode('utf-8'))} bytes, production={len(production.encode('utf-8'))} bytes, ids={len(production_ids)}")
