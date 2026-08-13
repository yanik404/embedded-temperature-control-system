# UI concept exploration

Datum: 13. August 2026  
Ausgangsstand: `ee7a704a44724d2da3529efd2d124a97eb9320af`

## Technische Leitplanken

- Echte Zustände stammen ausschließlich aus `/api/status`; Safety und START-Freigabe bleiben serverseitig.
- WLAN-Client, DHCP, lwIP Raw API, pbuf ownership und asynchrones HTTP-Streaming bleiben unverändert.
- Production bleibt ein einzelnes, offline ausgeliefertes Flash-Asset ohne CDN oder weitere HTTP-Requests.
- Animation läuft im Browser. Der Pico speichert weder Chart-Historie noch Events und erhält keine großen neuen Runtime-Buffer.
- Preview-Demodaten werden beim Build aus dem Production-Asset entfernt.

## Concept A — Spatial Technical Interface

- **Layout:** Temperaturzentrum in orbitaler Systemgeometrie, Zustandsrail links, Action-Orbit rechts.
- **Typografie/Farbe:** dünne Produkt-Typografie, cyanfarbene Telemetrie, nur aktive Wärme orange.
- **Bewegung/3D:** perspektivisches Bodengitter, rotierender Signalorbit, langsames thermisches Atmen.
- **Chart/Regelkreis:** Daten und Regelgrößen liegen auf konzentrischen Signalbahnen.
- **Controls:** räumlicher Action-Kreis statt Formularzeile.
- **Mobile:** zentrale Orbit-Zone mit vertikaler Systemrail.
- **Stärke:** hohe Eigenständigkeit und sehr gute Projektorwirkung.
- **Risiko:** Prozesshardware und Bedienweg bleiben zu abstrakt.

## Concept B — 3D Thermal Product UI

- **Layout:** große technische Schnittdarstellung von Becher, Platte, Peltier und Lüfter; Messwerte sind direkt an der Baugruppe annotiert.
- **Typografie/Farbe:** dunkle Produktbühne, warme Materialfarbe nur entlang des realen Energiepfads.
- **Bewegung/3D:** pseudo-räumliche Baugruppe, aufsteigende thermische Konturen, aktiver Peltier-Layer.
- **Chart/Regelkreis:** Prozessgrößen werden zuerst am Produkt gezeigt; die mathematische Ebene folgt darunter.
- **Controls:** großer state-aware Prozesskontrollkreis.
- **Mobile:** Baugruppe wird zur vertikalen Schnittzeichnung, Controls liegen direkt darunter.
- **Stärke:** der Nutzer versteht sofort, welches reale Produkt geregelt wird.
- **Risiko:** ohne wissenschaftliche Ebene könnte es mehr Produktshow als Regelungstechnik wirken.

## Concept C — Minimal Future Automotive

- **Layout:** ein großer thermischer Horizont mit zentraler Temperatur, sehr wenige Nebeninformationen.
- **Typografie/Farbe:** fast monochrom, helle Fläche, eine warme Fortschrittslinie.
- **Bewegung/3D:** kaum 3D; ruhige Arc-Interpolation und minimale Zustandswechsel.
- **Chart/Regelkreis:** stark abstrahiert als untere Signalzeile.
- **Controls:** klarer kreisförmiger STOP/START-Control.
- **Mobile:** hervorragend reduzierbar auf einen vertikalen Instrumentencluster.
- **Stärke:** höchste Lesbarkeit und geringste visuelle Last.
- **Risiko:** zu wenig Tiefe für Hardware, Safety und regelungstechnische Präsentation.

## Concept D — Scientific Control Field

- **Layout:** thermisches Konturfeld links, großflächige Messkurven und Regelabweichung rechts.
- **Typografie/Farbe:** wissenschaftlich-monospace, Cyan für Messung, Gelbgrün für Referenz, Koralle für Energie.
- **Bewegung/3D:** Feldlinien und Live-Cursor; keine dekorative 3D-Szene.
- **Chart/Regelkreis:** stärkste mathematische und wissenschaftliche Darstellung aller Konzepte.
- **Controls:** bewusst nachgeordnet; Engineering steht im Vordergrund.
- **Mobile:** Feld und Plot werden nacheinander als vertikale Messflächen gezeigt.
- **Stärke:** maximale Glaubwürdigkeit für einen Elektrotechnik-Dozenten.
- **Risiko:** für normale Benutzer weniger selbsterklärend und emotional.

## Concept E — Transparent Layered Engineering

- **Layout:** explodierte Hardware-Layer links, Systembus und Baugruppentelemetrie rechts.
- **Typografie/Farbe:** technische Annotationen, cyanfarbene Sensorik, orangefarbener Energiepfad.
- **Bewegung/3D:** räumlich getrennte Hardwareebenen und senkrechter Signal-/Energiefluss.
- **Chart/Regelkreis:** Regelkette als unterer durchgehender Systembus.
- **Controls:** separates Prozess-Overlay statt Bedienkarte.
- **Mobile:** vertikale Explosionszeichnung mit aufklappbarer Baugruppeninspektion.
- **Stärke:** unverwechselbare und fachlich plausible Hardwaredarstellung.
- **Risiko:** als Hauptansicht liegt der Fokus zu stark auf Elektronik statt Temperatur.

## Vergleich

Bewertungsskala 1–10, anhand der gerenderten 1920×1080-Prototypen:

| Kriterium | A | B | C | D | E |
|---|---:|---:|---:|---:|---:|
| WOW-Effekt | 9 | 10 | 7 | 8 | 9 |
| Technische Glaubwürdigkeit | 8 | 9 | 8 | 10 | 10 |
| Bedienbarkeit | 7 | 8 | 10 | 7 | 8 |
| Einzigartigkeit | 9 | 9 | 8 | 9 | 10 |
| Regelkreis-Verständlichkeit | 8 | 8 | 6 | 10 | 8 |
| Präsentationswirkung | 9 | 10 | 9 | 9 | 9 |
| Embedded-Übertragbarkeit | 9 | 9 | 10 | 9 | 9 |

## Gewählte Richtung: Thermal Product Observatory

Concept B bildet die primäre visuelle Richtung. Es erfüllt als einziges Konzept gleichzeitig den unmittelbaren Produktbezug, den thermischen Prozess und den gewünschten ersten WOW-Moment. Die Seite wird nicht als Dashboard, sondern als fortlaufende technische Beobachtungsumgebung aufgebaut.

Drei bewusst begrenzte Ergänzungen werden übernommen:

1. **Thermal Assembly** aus B bleibt Hero und visuelle Signatur.
2. **Scientific Signal Landscape** aus D prägt Regelkreis und Chart.
3. **Exploded Hardware Constellation** aus E ersetzt die bisherige Sammlung gleichartiger Hardwarekarten.

Aus A wird nur das Prinzip räumlicher Signalbahnen übernommen, aus C die typografische Zurückhaltung. Damit bleibt B klar erkennbar die gewählte Richtung; die Ergänzungen lösen konkrete Inhaltsprobleme, ohne fünf Stile zu mischen.

## WebGL-/Canvas-/SVG-Entscheidung

Three.js wurde ernsthaft gegen eine kleine eigene WebGL-Schicht abgewogen. Für diese Szene wären Szenengraph, Loader, Materialsystem und große Bibliotheksanteile unnötig. Eine Preview-only-Three.js-Lösung würde außerdem Feature-Parität und Performance-Messung verfälschen.

Die gewählte Architektur:

- ein sehr kleiner WebGL-Fragmentshader für das thermische Umgebungsfeld;
- ein Canvas-2D-Fallback, falls WebGL fehlt;
- SVG für die semantisch wichtigen Produkt-, Regelkreis- und Hardwarelinien;
- Canvas 2D für den Live-Verlauf;
- CSS nur für räumliche Komposition, Typografie und kontrollierte Microinteractions.

Damit entsteht eine echte GPU-Visualisierung mit einem Draw Call pro Frame, ohne externe Library. Die Production-Variante bleibt offline, klein und stabil.

## Motion-System

- **fast / 150 ms:** Fokus, Tasterfeedback, kleine Statusreaktionen;
- **normal / 300 ms:** Werte, Controls und Disclosure;
- **system / 650 ms:** Zustandswechsel und Recovery;
- **ambient / 3–8 s:** thermisches Feld, Wärmeaufstieg und Signalfluss.

Die WebGL-Schicht wird auf 30 FPS begrenzt, pausiert bei unsichtbarer Seite und zeichnet bei `prefers-reduced-motion` beziehungsweise Low-Motion nur statisch.
