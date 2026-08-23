# Embedded Temperature Control System

Firmware für die Praxisarbeit **Digitaltechnik & Regelungstechnik**. Ein Raspberry Pi Pico W heizt einen Becherhalter mit zwei vorhandenen Peltier-Kanälen kontrolliert auf eine einstellbare Solltemperatur auf und hält sie mit einem geschlossenen PI-Regelkreis. Eine responsive Weboberfläche stellt Regelkreis und Aufwärmverlauf live dar.

> Das System heizt ausschließlich. Es gibt keinen Kühlmodus und keine Richtungsumkehr der H-Brücken.

## Präsentationsmodus mit Handy-Hotspot

Der Pico W arbeitet als normaler WLAN-Client. Der Handy-Hotspot vergibt ihm automatisch per DHCP eine IPv4-Adresse; es gibt keine fest angenommene Pico-Adresse.

1. Handy-Hotspot einschalten.
2. SSID **Obi W-lan Kenobi** einstellen.
3. Passwort **xxxxxxxx** einstellen.
4. Hotspot auf **2,4 GHz** stellen.
5. Pico einschalten.
6. Der Pico verbindet sich automatisch mit dem Hotspot.
7. Die vom Hotspot vergebene IP auf dem OLED ablesen.
8. Laptop, Tablet oder weiteres Handy mit demselben Hotspot verbinden.
9. Browser öffnen.
10. **http://&lt;PICO-IP&gt;** eingeben, beispielsweise `http://192.168.43.117`.
11. Das Dashboard öffnet sich direkt vom Flash des Pico.

SSID und Passwort sind zentral in `include/secrets.h` hinterlegt. Ist der Hotspot vorübergehend nicht erreichbar, läuft die Firmware sicher weiter und versucht die Verbindung zeitgesteuert erneut. WLAN-Verbindung oder USB-Versorgung starten niemals die Heizung; ohne gültige Leistungsfreigabe bleibt START gesperrt.

## Sicherer Erststart

Nach jedem Reset werden beide Peltier-PWM-Ausgänge zuerst auf 0 % gesetzt, beide H-Brücken deaktiviert und der Lüfter in den AUS-Zustand gebracht. Die Initialisierung oder eine USB-Versorgung allein startet niemals die 12-V-Last. Heizen beginnt nur nach einem bewussten `START` über OK-Taster oder Weboberfläche und nur bei gültigen Sensoren, Strommessung, 5-V-Power-Good und erkannter Tasse.

Für den ersten Test ohne 12 V:

1. Firmware flashen und zunächst nur USB anschließen.
2. Den Handy-Hotspot einschalten, die IP auf dem OLED ablesen und `http://<PICO-IP>` öffnen.
3. OLED, Taster, beide Temperaturen, Lichtwert und Weboberfläche prüfen.
4. Nicht `START` drücken. Peltier-PWM und beide Richtungsleitungen müssen 0 sein.
5. Vor dem ersten 12-V-Test Heizrichtung und Stromskalierung gemäß Abschnitt „Kalibrierung“ prüfen.

## Hardware und feste Pinbelegung

| GPIO | Signal | GPIO | Signal |
|---:|---|---:|---|
| GP0 | `I2C_DATA` | GP1 | `I2C_CLOCK` |
| GP2 | `OUT_LED1_n` | GP3 | `OUT_LED2_n` |
| GP4 | `OUT_LED3_n` | GP5 | `HB_LED_n` |
| GP6 | `FB_LED_n` | GP7 | `PG_5V0` |
| GP8 | `RGB_DIN` | GP9 | `S_MODE` |
| GP10 | `S_DOWN` | GP11 | `S_OK` |
| GP12 | `S_UP` | GP13 | `S_DETECT` |
| GP14 | `FAN_PWM` | GP15 | `FAN_TACHO` |
| GP16 | `FAN_CTRL` | GP17 | `PEL1_INA` |
| GP18 | `PEL1_INB` | GP19 | `PEL1_PWM` |
| GP20 | `PEL1_SEL` | GP21 | `PEL2_INA` |
| GP22 | `PEL2_INB` | GP26 / ADC0 | `TEMP_1_ADC` |
| GP27 | `PEL2_PWM` | GP28 | `PEL2_SEL` |

TLA2024 an I2C0: AIN0 `PEL1_CS`, AIN1 `PEL2_CS`, AIN2 `LIGHT_ADC`, AIN3 `TEMP_2_ADC`. Ebenfalls am Bus: SSD1306 128×64, standardmäßig Adresse `0x3C`. Weitere Bauteile sind zwei VNH7070BASTR, zwei TMP36, 4-Pin-PWM-Lüfter, Lichtsensor, RGB-Ring und fünf Taster/Eingänge.

## Regelkreis

```text
Solltemperatur ──> PI-Regler ──> 0…100 % Heizleistung ──> Peltier / Becher
      ^                                                       │
      └──────── TMP36 <──── gefilterte Isttemperatur <────────┘
```

Der Regler berechnet `e = Sollwert - Istwert` alle 250 ms. Der PI-Ausgang wird auf 0…100 % begrenzt; negative Leistung ist unmöglich. Bedingte Integration verhindert Windup an beiden Ausgangsgrenzen. Der wärmere der beiden gültigen TMP36-Werte wird als konservative Regelgröße verwendet. `Kp`, `Ki`, Zykluszeit, Halteband und alle Temperaturgrenzen liegen zentral in `include/config.h`.

## Zustandsautomat

| Zustand | Verhalten |
|---|---|
| `OFF` / AUS | Heizleistung 0 %, Lasten sicher aus; per MODE dauerhaft wählbar |
| `READY` / BEREIT | Initialisiert und startbereit, noch keine Heizfreigabe |
| `HEATING` / AUFHEIZEN | PI-Regelung aktiv, Sollwert noch nicht erreicht |
| `HOLDING` / HALTEN | Sollwert im Halteband; PI liefert nur erforderliche Leistung |
| `ERROR` / FEHLER | Fehler verriegelt, beide Peltier-Kanäle sofort aus |

UP/DOWN ändern den Sollwert in 0,5-°C-Schritten. OK startet oder stoppt; im Fehlerzustand quittiert OK wie STOP, ein weiterhin vorhandener Fehler wird sofort erneut erkannt. MODE schaltet im inaktiven Betrieb zwischen AUS und BEREIT. `S_DETECT` ist aktiv-low als Bechererkennung implementiert.

## Sicherheitsfunktionen

- Peltier-Ausgänge werden als erste Hardwarefunktion sicher ausgeschaltet.
- Keine automatische Heizfreigabe und keine Kühlrichtung.
- Sollwertbereich 20…60 °C; unabhängige Maximaltemperatur 65 °C.
- ADC-, Temperatur- und Zwei-Sensor-Plausibilitätsprüfung mit Low-Pass-Filter.
- Verriegeltes `ERROR` bei Sensorfehler, Übertemperatur, Überstrom, unplausibler Strommessung, fehlendem Becher, Ausfall von 5-V-Power-Good während des Heizens oder Lüfterstillstand unter hoher Last. Bei USB-only darf Power-Good fehlen; START bleibt dann gesperrt.
- Lüfter-Mindestleistung bei aktiver Heizung, lastabhängige Drehzahl und 15 s Nachlauf.
- Hardware-Watchdog mit 3 s Timeout.
- Keine langen Delays im Hauptprogramm; alle Aufgaben werden per Zeitstempel geplant.

## Weboberfläche

Die aktuelle Oberfläche folgt dem Prinzip **Simple Digital Twin** und erklärt das System in dieser Reihenfolge:

1. **Aufbau:** hochwertige, sofort sichtbare Produktvisualisierung auf Basis der gelieferten STEP-Geometrie. **AUSSEN** zeigt das glaubwürdig fertig montierte Gerät mit mattem Gehäuse, tief eingesetztem Becher sowie OLED und vier physischen Tastern in der unteren Frontaussparung. **INNEN** ist bewusst als einfache interaktive Vektorgrafik aufgebaut: zwei kompakte Peltier-Kontakte, zwei TMP36, seitlicher Hebel-Mikroschalter, horizontaler Lüfter, PCB mit Pico W und ADC, befestigter Lichtsensor sowie das mittige Front-OLED bilden klar getrennte Ebenen. Über **GESAMT**, **THERMIK**, **SENSORIK** und **ELEKTRONIK** wird der gewünschte Funktionsbereich hervorgehoben. Bauteile bleiben anklickbar; auf Mobilgeräten stehen Produkt und Legende ohne Callout-Linien untereinander. Die Visualisierung benötigt weder WebGL noch externe Assets.
2. **Regelkreis und Regleranalyse:** Die linienbasierte Hauptgrafik zeigt SOLL → PI → HEIZEN → BECHER → IST mit sichtbarer Rückführung. Direkt darunter zerlegt eine Live-Grafik den echten PI-Ausgang in P- und I-Anteil, Rohwert, 0–100-%-Begrenzung, Stellgröße und Anti-Windup. Eine ausdrücklich als Modellannahme gekennzeichnete PT1/PT2-Darstellung erklärt die mögliche Streckenordnung; ein ausdrücklich nicht-live dargestellter P/PI/PID-Vergleich dient ausschließlich der regelungstechnischen Präsentation. Die Firmware bleibt ein PI-Regler ohne D-Anteil.
3. **Live:** eine dominante Isttemperatur, darunter Zustand sowie eine kompakte Zeile für Sollwert, Heizleistung und Lüfter. Es folgt unmittelbar der Verlauf von IST, SOLL und HEIZLEISTUNG; weitere Messreihen sind zunächst eingeklappt.
4. **Technik:** optionale Details zu GPIO, Messwerten, PI-Parametern, WLAN, IP und Firmwarestatus.

Live-Daten sind ohne Anmeldung lesbar. START und Sollwertänderungen sind zunächst gesperrt. Über **Mit PIN freigeben** wird die Präsentations-PIN `1234` an `POST /api/unlock` gesendet und ausschließlich auf dem Pico geprüft. Bei Erfolg erzeugt der Pico ein zufälliges, flüchtiges Token mit fünf Minuten Gültigkeit. START und Sollwert müssen dieses Token mitsenden; danach gelten unverändert sämtliche Sensor-, Becher-, Versorgungs-, Lüfter-, Strom- und Temperaturfreigaben der Firmware. **STOPP ist aus Sicherheitsgründen jederzeit ohne Token erlaubt**, damit eine abgelaufene Sitzung niemals eine sichere Abschaltung verhindert. Die PIN ist eine lokale Bedienfreigabe im vertrauenswürdigen Präsentationsnetz, keine verschlüsselte Benutzerverwaltung.

Der Pico W liefert die responsive Single-Page-Oberfläche direkt aus dem Flash unter `http://<PICO-IP>`. Normales Scrollen verbindet die vier Bereiche; es gibt kein Scroll-Jacking, keine Kamerafahrt und keinen X-Ray-/Engineering-Modus. Das Live-Diagramm aktualisiert sich alle 500 ms und hält bis zu 30 Minuten Verlauf ausschließlich im Browser. Die Produktion benötigt keine CDN-, Font- oder sonstigen Asset-Requests.

Statusfarben bleiben bewusst sparsam: Orange zeigt Heizenergie, Grün einen bestätigten sicheren Zustand, Rot einen echten Fehler, Blau Information und Grau einen unbekannten oder nicht angeschlossenen Zustand.

### Lokale Designvorschau

`preview.html` direkt im Browser öffnen, um dieselbe Oberfläche ohne Pico und WLAN mit animierten Demo-Daten anzusehen. Die Komponentenliste, die vier Innenmodi und die transparenten Hotspots bleiben interaktiv; Hinzufügen und **Aus Vorschau entfernen** ändern ausschließlich den lokalen Demo-Status. Die Außenansicht verwendet das freigestellte Produkt-Rendering, während die Innenansicht als bewusst reduzierte Vektorgrafik Becher, Heizmodule, Sensoren, Schalter, Lüfter, PCB/Pico, OLED und Lichtsensor klar trennt. Die Szenarien sind standardmäßig verborgen und öffnen sich über **DEMO TOOLS** oder `Alt+D`; in der Produktionsdatei werden diese Werkzeuge nicht erzeugt. Die simulierte PIN ist ebenfalls `1234`; START, STOP und Sollwert verändern ausschließlich den lokalen Demo-Zustand und senden niemals Hardware- oder API-Befehle.

Die bewährte Render-, API- und Chart-Basis liegt unter `ui-v3/src/`; die V4-Digital-Twin-Schicht liegt getrennt unter `ui-v4/src/`. Die Außenansicht bleibt fertigungsnah, die interaktive Innenansicht ist für bessere Lesbarkeit als einfache technische Vektorgrafik aufgebaut. Die gelieferten technischen STEP-Bilder bleiben als Legacy-Fallback erhalten. `ui/src/` bleibt als jederzeit wiederherstellbare Observatory-V2-Basis im Repository:

- `index.html` – semantische Oberfläche und SVG-Systemdarstellungen
- `ui-v3/src/product.svg` – unveränderter visueller Fallback; wird vom aktuellen Generator nicht eingebettet
- `experience.css` – vierstufige Simple-Digital-Twin-Komposition und responsive Layouts
- `experience.js` – Read-only-Live-API, Token-Bedienung, Chart und technische Details
- `preview.js` – ausschließlich lokale Simulation
- `ui-v4/src/component-model.js` – vollständiger Hardwarekatalog und ehrliche Live-Discovery-Abbildung
- `ui-v4/src/digital-twin.js` – Callout-Legende, SVG-Montageanker, ehrliche Live-Zustände und Preview-Konfigurator
- `ui-v4/src/digital-twin.css` – orthogonale Zuordnungslinien, Statussemantik und Inline-Fokusdarstellung
- `ui-v7/src/product-finished-exterior-v2.webp` – aktive transparente Außenansicht mit Peltier auf Becherhöhe sowie OLED und Tastern in der unteren Aussparung
- `ui-v7/src/product-finished-exterior.webp` – vorherige realistische Produktansicht als visueller Fallback
- `ui-v7/src/product-step-exterior.webp` – unveränderte technische STEP-Referenz aus `ThermoCup_Codex_Ready.zip`
- `ui-v7/src/product-v3-exterior.svg` – schlanker, responsiver Einbettungsrahmen für die Außenansicht
- `ui-v6/src/product-v3-rejected-fallback.svg` – unveränderter Fallback der verworfenen großen Maschinenform
- `ui-v5/src/product-v2-exterior.svg` – unveränderter visueller Legacy-Fallback
- `ui-v5/src/product-finished-cutaway-v2.webp` – erhaltene helle CAD-Schnittansicht als visueller Fallback
- `ui-v5/src/product-finished-cutaway.webp` – vorherige realistische Schnittansicht als visueller Fallback
- `ui-v5/src/product-step-cutaway.webp` – unveränderte technische Cutaway-Referenz mit originalem 80-mm-PCB und genau einem Lüfter
- `ui-v5/src/product-v2-cutaway.svg` – aktive, vollständig vektorielle Innenansicht mit klaren Bauteilformen und interaktiven Hotspots

Die stabilisierte Produktion bettet die Außenansicht als Data-URL und die Innenansicht direkt als Inline-SVG in die bestehende SVG-/DOM-/Canvas-Architektur ein. Dadurch bleibt das Dashboard eine einzelne Flash-Datei ohne zusätzliche HTTP-Asset-Requests. Frühere WebGL-, X-Ray-, Scroll-Mode- und Bedienlabor-Prototypen wurden nach Abschluss der Technologieentscheidung entfernt. Die aktuelle Architektur steht in `docs/ui-v3-architecture.md` und `docs/ui-v4-architecture.md`.

`tools/build_ui.py` erzeugt daraus `preview.html`, die minifizierte Single-File-Produktion unter `ui/dist/dashboard.production.html`, Größenstatistiken und `include/web_assets.h`. Die Produktion enthält keine Simulation, keine externen URLs, keine CDN-Abhängigkeit und benötigt für Assets keinen weiteren HTTP-Request. Die CMake-Firmware hängt vom Generator ab; manuell lässt er sich mit dem im CMake-Cache erkannten Python ausführen:

```powershell
python tools/build_ui.py
python tools/build_ui.py --check
```

`powershell -ExecutionPolicy Bypass -File tools/capture_ui_review.ps1 -Round manual` rendert die sieben Referenz-Viewports automatisiert mit exakt gesetzten Chrome-/Edge-DevTools-Gerätemetriken nach `build/ui-review/manual/`. `python tools/audit_ui_layout.py` prüft AUFBAU, REGELKREIS und LIVE bei denselben sieben Viewports auf sichtbare Kollisionen und horizontalen Overflow. Mit `tools/capture_ui_review.py --product-test product|scale20|grayscale|blur|silhouette` lässt sich die Produktgrafik reproduzierbar isolieren. `tools/prepare_product_cutaway.py` bleibt zur reproduzierbaren Erzeugung des früheren Cutaway-Fallbacks erhalten. `tests/test_v3_visual_contract.py` fixiert die SHA-256-Hashes der aktiven Produkt-Assets und der erhaltenen STEP-Fallbacks; `tests/test_product_v2_responsive.py` prüft Außenansicht und alle vier Innenmodi automatisiert bei 1920, 1440, 1366, 768 und 390 Pixeln.

## Softwarearchitektur

| Modul | Aufgabe |
|---|---|
| `app` | nichtblockierender Scheduler, Integration, State Machine |
| `controller` | PI-Regler und Anti-Windup |
| `temperature` | TMP36-Wandlung, Filter, Plausibilität |
| `peltier` | einzige GPIO/PWM-Abstraktion der Heizkanäle |
| `fan` | PWM, Tacho/RPM |
| `tla2024`, `current_measurement`, `light_sensor` | externer ADC und Messgrößen |
| `display` | SSD1306-Statusanzeige |
| `buttons` | nichtblockierende Entprellung und Bechererkennung |
| `status_leds` | aktive-low LEDs und WS2812-Ring |
| `safety` | zentrale Freigabe- und Fehlerprüfung |
| `webserver` | WLAN-Client mit DHCP und Reconnect, lwIP-HTTP-Server, JSON-API, Dashboard |

Der Pico nutzt den DHCP-Client von lwIP. Sobald die Station vollständig verbunden ist, wird die tatsächlich zugewiesene IPv4-Adresse aus dem STA-Netzwerkinterface gelesen und an OLED sowie Dashboard weitergegeben.

## Build und Flash

Voraussetzungen: Pico SDK 2.1.1 oder kompatibel, ARM GNU Toolchain, CMake ab 3.13 und ein Pico W.

```powershell
$env:PICO_SDK_PATH = 'C:\path\to\pico-sdk'
cmake -S . -B build -DPICO_BOARD=pico_w
cmake --build build --parallel
```

Beim CMake-Build werden die Preview- und Embedded-Webassets automatisch und deterministisch aktualisiert. `tests/test_ui_build.py` und der Workflow `.github/workflows/ui.yml` verhindern, dass lesbare Quellen, Produktions-HTML und C-Header auseinanderlaufen.

Zum Flashen BOOTSEL gedrückt halten, Pico per USB verbinden und `build/temperature_control.uf2` auf das erscheinende Laufwerk kopieren. Der CI-Workflow baut dieselbe UF2-Datei und stellt sie als Artefakt bereit.

## Kalibrierung vor 12-V-Betrieb

Das ursprüngliche Repository enthielt keine Hardwaredokumente. Deshalb müssen folgende Werte am aufgebauten Board verifiziert werden:

1. Mit strombegrenztem Netzteil und kleiner Leistung bestätigen, dass `PELTIER_HEAT_INA_LEVEL=1` / `INB=0` tatsächlich die Becherplatte erwärmt. Bei falscher Richtung ausschließlich diese beiden Konstanten anpassen – niemals dynamisch umkehren.
2. `CURRENT_AMPS_PER_VOLT`, `CURRENT_ZERO_V` und `CURRENT_MAX_A` anhand VNH7070-Multisense-Beschaltung und Referenzmessgerät kalibrieren.
3. Logikpegel von `S_DETECT`, `PG_5V0`, `FAN_CTRL` und `PELx_SEL` gegen Schaltplan/PCB prüfen.
4. PI-Parameter zunächst mit Wasserlast und konservativem Sollwert (z. B. 35 °C) auf wenig Überschwingen abstimmen.
5. Lüfter-Tachofaktor und sichere Temperaturgrenze praktisch validieren.

## Testablauf

1. USB-only Boot: alle Leistungsausgänge bleiben aus.
2. Offene/kurzgeschlossene Temperatursensoren: `ERROR`, Peltier aus.
3. START ohne Becher: keine Freigabe.
4. STOP aus AUFHEIZEN/HALTEN: PWM sofort 0 %, Lüfternachlauf aktiv.
5. Mit strombegrenzten 12 V langsam auf 35 °C aufheizen; Ist, Soll, Abweichung und Leistung im Dashboard beobachten.
6. Lüfter-Tacho unter hoher Leistung trennen: nach Gnadenzeit `ERROR`.
7. Strom- und Übertemperaturabschaltung mit sicheren, simulierten Messwerten prüfen.
8. Langzeittest im HALTEN durchführen und PI-Parameter dokumentiert feinabstimmen.

Die hostseitigen Tests prüfen Begrenzung, reinen Heizbetrieb und Anti-Windup. GitHub Actions kompiliert zusätzlich die vollständige Pico-W-Firmware mit Warnungsprüfung des Hosttests.
