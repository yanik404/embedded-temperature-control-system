# Embedded Temperature Control System

Firmware für die Praxisarbeit **Digitaltechnik & Regelungstechnik**. Ein Raspberry Pi Pico W heizt einen Becherhalter mit zwei vorhandenen Peltier-Kanälen kontrolliert auf eine einstellbare Solltemperatur auf und hält sie mit einem geschlossenen PI-Regelkreis. Eine responsive Weboberfläche stellt Regelkreis und Aufwärmverlauf live dar.

> Das System heizt ausschließlich. Es gibt keinen Kühlmodus und keine Richtungsumkehr der H-Brücken.

## Präsentationsmodus ohne externes WLAN

Der Pico W erzeugt beim Start selbst ein WPA2-geschütztes WLAN und stellt per DHCP automatisch Netzwerkadressen für mehrere verbundene Geräte bereit. Ein Internetzugang oder bestehender WLAN-Router ist nicht erforderlich.

1. Pico einschalten.
2. Auf Handy oder PC mit dem WLAN **Becherhalter** verbinden.
3. Passwort **12345678** eingeben.
4. Browser öffnen.
5. **http://192.168.4.1** aufrufen.

Dieses WLAN hat kein Internet. Das ist beabsichtigt, weil der Pico selbst die Webseite bereitstellt. Smartphone und PC können gleichzeitig verbunden sein. Der Access Point funktioniert auch bei reiner USB-Versorgung; dies startet niemals die Heizung und ohne gültige Leistungsfreigabe bleibt START gesperrt.

## Sicherer Erststart

Nach jedem Reset werden beide Peltier-PWM-Ausgänge zuerst auf 0 % gesetzt, beide H-Brücken deaktiviert und der Lüfter in den AUS-Zustand gebracht. Die Initialisierung oder eine USB-Versorgung allein startet niemals die 12-V-Last. Heizen beginnt nur nach einem bewussten `START` über OK-Taster oder Weboberfläche und nur bei gültigen Sensoren, Strommessung, 5-V-Power-Good und erkannter Tasse.

Für den ersten Test ohne 12 V:

1. Firmware flashen und zunächst nur USB anschließen.
2. Mit dem WLAN **Becherhalter** verbinden und `http://192.168.4.1` öffnen.
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

Der Pico W liefert das Dashboard direkt aus dem Flash unter `http://192.168.4.1` aus. Es zeigt Ist-/Solltemperatur, Regelabweichung, Leistung, Zustand, Lüfter-RPM, beide Ströme, Sensor-, System- und WLANstatus. Das Live-Blockdiagramm enthält die aktuellen Werte in den Regelkreisblöcken und eine sichtbare Rückführung. Ein Canvas-Diagramm aktualisiert Istwert, Sollwert und Leistung alle 500 ms ohne Seitenreload. START, STOP und Sollwert sind am PC und Smartphone bedienbar.

### Lokale Designvorschau

`preview.html` direkt im Browser öffnen, um dieselbe Oberfläche ohne Pico und WLAN mit animierten Demo-Daten anzusehen. START, STOP und SETZEN verändern ausschließlich den lokalen Demo-Zustand und sprechen keine Hardware oder Netzwerk-API an.

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
| `webserver`, `dhcpserver` | WPA2-Access-Point, DHCP, lwIP-HTTP-Server, JSON-API, Dashboard |

Der DHCP-Server basiert auf dem offiziellen Raspberry-Pi-Beispiel [`pico_w/wifi/access_point`](https://github.com/raspberrypi/pico-examples/tree/master/pico_w/wifi/access_point). Der übernommene MicroPython-DHCP-Code steht unter der MIT-Lizenz; der Lizenztext liegt in `third_party/dhcpserver/LICENSE`.

## Build und Flash

Voraussetzungen: Pico SDK 2.1.1 oder kompatibel, ARM GNU Toolchain, CMake ab 3.13 und ein Pico W.

```powershell
$env:PICO_SDK_PATH = 'C:\path\to\pico-sdk'
cmake -S . -B build -DPICO_BOARD=pico_w
cmake --build build --parallel
```

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
