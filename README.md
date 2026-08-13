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

Der Pico W liefert die responsive V4 **Configurator / Digital Twin Experience** direkt aus dem Flash unter `http://<PICO-IP>` aus. Der Becherhalter ist die Oberfläche: Ein eigener WebGL-Raymarcher modelliert ein leicht konisches Trinkglas mit Inhalt, zwei seitlich kontaktierte TMP36, Metallplatte, zwei Peltiermodule, Lamellenkühlkörper, Frontlüfter, befestigte Pico-/TLA2024-Platine sowie Bedienmodul in kontrollierter 3/4-Perspektive. Beleuchtung, Tiefennebel, Wärmematerial, Luft-/Energiefluss und die Explosionsdarstellung reagieren auf reale API-Werte. Ohne WebGL bleibt automatisch dieselbe mechanische Anordnung als semantische SVG-Produktrückfallebene sichtbar.

Der geschlossene Regelkreis läuft räumlich um das Produkt: `w(t)` → Vergleich → PI → `u(t)` → Peltier → Becher/Strecke → TMP36 `y(t)` → Rückführung. Ein transparenter Canvas-Zeithorizont aktualisiert Istwert, Sollwert, zweite Temperatur und Stellgröße alle 500 ms ohne Seitenreload; die Zeitfenster 1, 5 und 15 Minuten verwenden ausschließlich browserlokale Historie. Die minimale Randzeile zeigt SSID, aktuelle DHCP-IP, Live/Offline und Zeit des letzten Signals.

Die Ansichten `PRODUKT`, `REGELUNG`, `THERMISCH` und `ENGINEERING` beantworten bewusst verschiedene Fragen. PRODUCT erklärt das Gerät, CONTROL macht den geschlossenen Kreis verständlich, THERMAL zeigt den Wärmeweg und ENGINEERING fährt den Aufbau auseinander. Die zusätzlichen Linsen `PRODUCT`, `X-RAY` und `SIGNALS` wechseln zwischen ruhigem Produktbild, transparentem technischem Aufbau und elektrischen Signalpfaden. Eine durchgehende Scroll-Story führt vom physischen System über Regelkreis, Live-Control und Analyse bis zu Safety und Engineering, ohne das Produkt als sichtbare Regelstrecke zu ersetzen.

Elegante `+`-Hotspots fokussieren eine Komponente, ihren Signalweg, Pin/Kanal und den realen Live-Wert. Ihre Positionen entstehen aus den 3D-Weltkoordinaten und derselben Kamera wie das Produkt; sie bleiben dadurch auch bei Kamerabewegung und Explosionsansicht am Bauteil. Die Detailansicht trennt `KONFIGURATION`, `VERBINDUNG`, `LIVE` und `MESSWERT`. Der Digital Twin unterscheidet dabei strikt `VORGESEHEN`, `ANGESCHLOSSEN`, `ERKANNT`, `AKTIV`, `MESSUNG GÜLTIG`, `NICHT ERKANNT`, `NICHT DIREKT ÜBERWACHT`, `NICHT VERFÜGBAR` und `FEHLER`. Nicht angeschlossene optionale Hardware wird nicht automatisch als Systemfehler ausgegeben. `SYSTEM ERKLÄREN` führt in sechs animierten Schritten durch Messung, Vergleich, PI-Regler, Aktor, Regelstrecke und Rückführung.

Der interaktive Analysehorizont kann `TEMP 1`, `TEMP 2`, `SOLL`, `u(t)`, `I1`, `I2`, `FAN` und `LIGHT` ein- oder ausblenden. Nicht sinnvoll verfügbare Signale werden nicht angeboten. Ein PI-Fokus zeigt Kp, Ki, P-/I-Anteil und Anti-Windup read-only. Der Vollbild-Präsentationsmodus priorisiert Produkt, Regelkreis und Live-Verlauf. Bei ausbleibenden API-Antworten bleiben Szene und letzter Verlauf sichtbar, während alle Aktionen gesperrt werden. START, STOP und Sollwert bleiben auf PC, Tablet und Smartphone bedienbar. Die Safety-Entscheidung bleibt immer serverseitig.

Der Hardwarestatus unterscheidet bewusst zwischen `OK`, `AKTIV`, `AUS`, `FEHLER`, `NICHT VERBUNDEN`, `NICHT VERFÜGBAR` und `UNBEKANNT`. Ein grüner Status wird nur angezeigt, wenn die Firmware die Initialisierung, einen plausiblen Messwert oder einen beobachtbaren Betriebswert bestätigen kann. Die Startfreigabe nennt bei einer Sperre den konkreten Grund, beispielsweise einen fehlenden Becher, eine ungültige Strommessung oder fehlendes Power-Good.

Die Statusfarben bedeuten: Grün = bestätigt/aktiv, Blau = normal/bereit, Orange = Warnung oder unbekannt, Rot = Fehler und Grau = aus oder nicht verfügbar.

### Lokale Designvorschau

`preview.html` direkt im Browser öffnen, um exakt dieselbe Oberfläche ohne Pico und WLAN mit animierten Demo-Daten anzusehen. START, STOP und SOLLWERT ÜBERTRAGEN verändern dort ausschließlich den lokalen Demo-Zustand. `SYSTEMAUFBAU` öffnet den lokalen Konfigurator: Alle 19 Komponenten bleiben als geplante Systemkonfiguration sichtbar; nur ihr simulierter Anschlusszustand wird verändert. Das führt niemals einen Hardware- oder API-Befehl aus. Die Preview-Profile `FULL SYSTEM`, `MINIMAL SYSTEM`, `PARTIAL HARDWARE` und `NO SENSORS` demonstrieren die Trennung von `VORGESEHEN`, `NICHT ANGESCHLOSSEN` und Live-Erkennung. Die Szenarien `READY`, `HEATING`, `HOLDING`, `ERROR`, `OFFLINE`, `RECONNECT`, `SENSOR ERROR`, `FAN ERROR`, `POWER ERROR` und `30 MIN DEMO` prüfen Betriebszustände sowie einen vollständigen Aufheiz-/Halteverlauf von 22 auf 50 °C.

Die bewährte Render-, API- und Chart-Basis liegt unter `ui-v3/src/`; die V4-Digital-Twin-Schicht liegt getrennt unter `ui-v4/src/`. `ui/src/` bleibt als jederzeit wiederherstellbare Observatory-V2-Basis im Repository:

- `index.html` – semantische Oberfläche und SVG-Systemdarstellungen
- `experience.css` – räumliche Komposition, Zustände, Responsive- und Motion-System
- `product-scene.js` – eigener 3D-SDF/WebGL-Renderer mit einem Draw Call
- `thermal-overlay.js` – dezente Wärme-, Partikel- und Luftstromebene
- `experience.js` – reale API, Bedienung, Chart und technische Ansichten
- `preview.js` – ausschließlich lokale Simulation
- `ui-v4/src/component-model.js` – vollständiger Hardwarekatalog und ehrliche Live-Discovery-Abbildung
- `ui-v4/src/digital-twin.js` – Hotspots, Komponentenfokus, Konfigurator, Scroll-Story und Guided Journey
- `ui-v4/src/digital-twin.css` – produktzentrierte V4-Komposition für Desktop, Tablet und Mobile

Die ausführbaren Vergleiche unter `ui-v3/prototypes/` enthalten einen echten Three.js-Prototyp, einen Custom-WebGL-Prototyp und drei funktionierende Sollwert-Bedienkonzepte. V3-Technologieentscheidung und V4-Digital-Twin-Architektur stehen in `docs/ui-v3-architecture.md` und `docs/ui-v4-architecture.md`.

`tools/build_ui.py` erzeugt daraus `preview.html`, die minifizierte Single-File-Produktion unter `ui/dist/dashboard.production.html`, Größenstatistiken und `include/web_assets.h`. Die Produktion enthält keine Simulation, keine externen URLs, keine CDN-Abhängigkeit und benötigt für Assets keinen weiteren HTTP-Request. Die CMake-Firmware hängt vom Generator ab; manuell lässt er sich mit dem im CMake-Cache erkannten Python ausführen:

```powershell
python tools/build_ui.py
python tools/build_ui.py --check
```

`powershell -ExecutionPolicy Bypass -File tools/capture_ui_review.ps1 -Round manual` rendert die sieben Referenz-Viewports automatisiert mit exakt gesetzten Chrome-/Edge-DevTools-Gerätemetriken nach `build/ui-review/manual/`. `python tools/audit_ui_layout.py` prüft zusätzlich alle acht Ansichten bei denselben sieben Viewports auf sichtbare Text-/Hotspot-Kollisionen, abgeschnittene Bedienelemente und horizontalen Overflow. Konzeptentscheidung, visuelle Reviews, Persona-/Anti-Card-Audit und Performancewerte sind unter `docs/` dokumentiert.

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
