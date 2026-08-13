# Callout-Konfigurator: visuelle Qualitätsprüfung

## Gesicherte Ausgangslage

- Branch: `feature`
- Referenz-Commit: `efe288a443695884badf38eb22ee2d395644f724`
- Referenz-Viewport: 1920 x 1080
- Gesamtansicht: `build/ui-review/baseline-efe288a-full/1920x1080.png`
- Aufbau: `build/ui-review/baseline-efe288a-aufbau/1920x1080.png`
- Regelkreis: `build/ui-review/baseline-efe288a-loop/1920x1080.png`
- Live: `build/ui-review/baseline-efe288a-live/1920x1080.png`

Die Produktzeichnung ist die verbindliche Qualitätsbasis. Der folgende Umbau
ändert ihre technische Identität nicht, sondern ordnet Auswahl, Status und
Bauteilzuordnung neu.

## Review Runde 1 – 25 konkrete Befunde

1. Die Produktzeichnung belegt links nicht klar genug die angestrebten 65–70 %.
2. Der rechte Bereich wirkt wie ein Dashboard, nicht wie eine technische Legende.
3. Systemstatus, Isttemperatur und Heizleistung wiederholen Inhalte aus LIVE.
4. Die erklärende Reglerzeile konkurriert mit der Komponentenbedienung.
5. Der permanente Bauteil-Fokus beansprucht viel Höhe, auch ohne Nutzereingriff.
6. Die Fokusfläche zeigt drei Zustände gleichzeitig und erzeugt unnötige Dichte.
7. Die Komponentenliste wiederholt die Funktion der Punkte auf der Zeichnung.
8. Das zweispaltige Kartenraster erschwert eine klare vertikale Zuordnung.
9. Die Kartenrahmen erzeugen zwölf zusätzliche sichtbare Kanten.
10. Plus-/Haken-Bedienpunkte liegen direkt über technischen Bauteilen.
11. Die Bedienpunkte verdecken Details der bereits hochwertigen Illustration.
12. Ein sichtbarer Haken wird fälschlich Teil der Produktzeichnung.
13. Die räumliche Zuordnung zwischen Listenzeile und Einbauort fehlt.
14. Sensor 1 und Sensor 2 sind ohne räumliche Führung schwer unterscheidbar.
15. Peltier links und rechts sind textlich, aber nicht geometrisch zugeordnet.
16. Der Lüfter ist gut erkennbar, besitzt aber keine geführte Listenbeziehung.
17. OLED und Taster werden als getrennte Bauteile nicht präzise adressiert.
18. PCB/Pico, Strommessung, Licht und Bechererkennung fehlen in der Kurzliste.
19. Die Komponenten-Gruppierung nach Funktion ist nicht sichtbar.
20. Fehlende Bauteile besitzen keine ruhige Montage-Silhouette.
21. Hover und Fokus erklären die Baugruppe nicht direkt in der relevanten Zeile.
22. Die dauerhaft sichtbare Preview-Leiste verfälscht jede Präsentationsaufnahme.
23. Der Regelkreis koppelt Peltier und Becher in einem gemeinsamen Hauptschritt.
24. LIVE verwendet neben dem Temperaturhelden noch ein unnötiges 2×2-Werteraster.
25. Die mobile Ansicht müsste Bedienelemente auf dem Produkt zusammendrängen.

## Zu prüfende Linienvarianten

- A: alle Zuordnungslinien dauerhaft dezent sichtbar
- B: nur die Linien der aktiven Funktionsgruppe sichtbar
- C: nur die fokussierte, per Tastatur gewählte oder überfahrene Zeile sichtbar

Die Varianten werden bei identischem Zustand und identischem 1920er-Viewport
aufgenommen. Gewählt wird die leiseste Variante, die die Einbauposition ohne
Kreuzungen, Textüberlagerungen oder dekorative Unruhe eindeutig erklärt.

## Variantenvergleich

- A: `build/ui-review/callout-variant-a-all/1920x1080.png`
- B: `build/ui-review/callout-variant-b-group/1920x1080.png`
- C: `build/ui-review/callout-variant-c-focus/1920x1080.png`

Gewählt wurde **Variante C**. A erzeugt trotz geringer Deckkraft ein dichtes
Leitungsnetz. B ist ruhiger, lenkt aber weiterhin drei parallele Beziehungen
gleichzeitig durch die Illustration. C zeigt genau eine überprüfbare Beziehung,
vermeidet Linienkreuzungen vollständig und lässt die Produktzeichnung dominant.
Die Route liegt hinter der Produktzeichnung; opake Produktteile verdecken sie,
statt dass eine Linie sichtbar durch Becher oder Baugruppen läuft. Nur der kleine
Montageanker liegt direkt auf dem Bauteil.
Die beiden dichteren Modi bleiben nur über `?callouts=all` beziehungsweise
`?callouts=group` für reproduzierbare Reviews erreichbar; Standard ist
`?callouts=focus`.

## Review Runde 2 – 15 Befunde nach dem Strukturumbau

1. Die Produktzeichnung ist nun eindeutig die größte visuelle Fläche.
2. Die rechte Seite liest sich als technische Legende statt als zweites Dashboard.
3. Systemzustand und Komponentenanzahl sind auf eine kompakte Kopfzeile reduziert.
4. Die fünf Fachgruppen bilden eine schnell erfassbare Informationshierarchie.
5. Alle zwölf relevanten Montagebeziehungen sind textlich verfügbar.
6. Variante A ist wegen des sichtbaren Leitungsnetzes zu unruhig.
7. Variante B ist besser, zeigt aber noch zu viele gleichzeitige Beziehungen.
8. Variante C besitzt keine Linienkreuzung, weil nur eine Beziehung aktiv ist.
9. Die lange Route zu linken Baugruppen bleibt bewusst am Produktrand.
10. Die Linien-Deckkraft kann für große Monitore noch etwas reduziert werden.
11. Der Zielring bleibt klein genug, um kein neuer Hotspot zu werden.
12. Inline-Details bleiben auf Typ, Rolle und Live-Wert beschränkt.
13. Plus, Haken, Gedankenstrich und Fehlerzeichen besitzen getrennte Semantik.
14. Ein unbekannter API-Wert darf die Hardware nicht als fehlend darstellen.
15. Die lokale Szenarioleiste ist jetzt standardmäßig vollständig verborgen.

## Regelkreis-Review – 15 Prüfpunkte

1. SOLL bleibt der eindeutige Eingang.
2. Der Vergleich ist kleiner und visuell sekundär.
3. e(t) bleibt unmittelbar am Vergleich lesbar.
4. PI ist als eigener Reglerblock erhalten.
5. HEIZUNG benennt den Aktor eindeutig.
6. u(t) steht direkt bei der Peltier-Heizung.
7. Das Peltier-Symbol ist aus der Produktgeometrie abgeleitet.
8. BECHER ist jetzt eine eigene Regelstrecke.
9. G(s) steht direkt am Becher.
10. Der Sensor ist als realer TO-92-Typ abstrahiert.
11. IST und y(t) stehen zusammen am Ausgang.
12. Es existiert genau eine blaue Rückführung.
13. Die Hauptflussrichtung bleibt von links nach rechts.
14. Der Bereich verwendet keine Karten oder dekorativen Boxen.
15. Die vertikale Mobilanordnung bewahrt dieselbe Reihenfolge.

## Live-Review – 10 Prüfpunkte

1. Die Isttemperatur ist die einzige übergroße Zahl.
2. Der Zustand steht direkt unter der Temperatur.
3. Soll, Heizleistung und Lüfter bilden genau eine kompakte Zeile.
4. Der Sensorstatus bleibt klein und sekundär.
5. Das 2×2-Werteraster ist entfernt.
6. Der Verlauf folgt unmittelbar auf den Messwertblock.
7. IST, SOLL und HEIZLEISTUNG sind standardmäßig aktiv.
8. Zusätzliche Signale bleiben eingeklappt.
9. Die gesperrte Steuerung liegt weiterhin unter dem Diagramm.
10. STOPP und die serverseitige PIN-/Token-Freigabe bleiben unverändert.

## Finale Prüfreferenzen

- Produkt 1920: `build/ui-review/callout-final-product/1920x1080.png`
- Ruhiger Aufbau ohne Auswahl: `build/ui-review/callout-final-product-clean/1920x1080.png`
- Produkt 1440: `build/ui-review/callout-final-product/1440x900.png`
- Produkt 1366: `build/ui-review/callout-final-product/1366x768.png`
- Produkt 1024: `build/ui-review/callout-final-product/1024x768.png`
- Produkt 768: `build/ui-review/callout-final-product/768x1024.png`
- Produkt 430: `build/ui-review/callout-final-product/430x932.png`
- Produkt 390: `build/ui-review/callout-final-product/390x844.png`
- Regelkreis: `build/ui-review/callout-final-loop/1920x1080.png`
- Live: `build/ui-review/callout-final-live/1920x1080.png`
- Produkt ohne Legende: `build/ui-review/callout-product-alone/product-alone.png`
- Silhouette bei 20 %: `build/ui-review/callout-product-alone/product-alone-20-percent.png`
- Layoutaudit: `build/ui-review/callout-final-layout-audit.json`

Der 21-fache Audit über AUFBAU, REGELKREIS und LIVE bei allen sieben
Pflicht-Viewports meldet jeweils null Kollisionen und null horizontalen Overflow.
Die 20-%-Miniatur bleibt als Becherhalter mit seitlichen Heizmodulen,
Elektronikbasis, Frontdisplay und Lüfter eindeutig erkennbar.

## 40 konkrete visuelle Verbesserungen

1. Produktanteil auf ungefähr zwei Drittel der Aufbaufläche erhöht.
2. Rechte Spalte als technische Legende statt Dashboard formuliert.
3. Doppelte Isttemperatur aus AUFBAU entfernt.
4. Doppelte Heizleistung aus AUFBAU entfernt.
5. Regler-Erzähltext aus der Komponentenbedienung entfernt.
6. Permanente Bauteil-Fokuskarte entfernt.
7. Sechs UI-Hotspots auf der Illustration entfernt.
8. Zweispaltiges Komponenten-Kartenraster entfernt.
9. Fünf fachlich benannte Baugruppen eingeführt.
10. Zwölf konkrete Montagebeziehungen aufgenommen.
11. Listenstatus auf kleine, eindeutige Zeichen reduziert.
12. Plus ausschließlich für fehlende Vorschauhardware verwendet.
13. Haken ausschließlich für vorhandene Hardware verwendet.
14. Gedankenstrich für unbekannte Live-Daten verwendet.
15. Rot ausschließlich für echte Fehler reserviert.
16. Montageanker als kleine neutrale Ringe ausgeführt.
17. Orthogonale statt diagonale Callout-Routen eingesetzt.
18. Linke Bauteile über eine äußere Randspur angebunden.
19. Linien hinter der Produktzeichnung angeordnet.
20. Standardansicht auf genau eine aktive Linie begrenzt.
21. Hover koppelt Zeile, Linie und reales Bauteil.
22. Tastaturfokus koppelt Zeile, Linie und reales Bauteil.
23. Inline-Details auf Typ, Rolle und Live-Wert begrenzt.
24. Detailansicht direkt unter der gewählten Zeile platziert.
25. Hinzufügen mit kurzer Montagebewegung visualisiert.
26. Linie beim Hinzufügen für ungefähr 300 ms bestätigt.
27. Entfernen als explizite Kontextaktion angeboten.
28. Fehlende Montagepositionen gestrichelt dargestellt.
29. Lüfter um sichtbare vieradrige Leitung ergänzt.
30. Produkt-Kopfstatus auf Zustand und Anzahl reduziert.
31. DEMO TOOLS standardmäßig vollständig verborgen.
32. Vorschauwerkzeuge aus der Produktion ausgeschlossen.
33. Peltier-Heizung im Regelkreis als eigener Aktor gezeigt.
34. Becher im Regelkreis als eigene Strecke gezeigt.
35. Vergleichsknoten visuell sekundär gesetzt.
36. Eine einzige blaue Rückführung bewahrt.
37. Live-2×2-Raster durch eine Messwertzeile ersetzt.
38. Isttemperatur als einzige dominante Live-Zahl gesetzt.
39. Leader Lines auf Mobilgeräten vollständig entfernt.
40. Produkt und Legende auf Mobilgeräten linear angeordnet.
