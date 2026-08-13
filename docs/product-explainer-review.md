# Simple Interactive Product Explainer – visueller Qualitätslauf

## Designentscheidung

Die Produktgeometrie ist keine primitive Raymarching-/WebGL-Szene mehr. Eine semantische Inline-SVG ist nun die primäre und einzige Produktdarstellung. Sie ist im ersten HTML-Frame vollständig sichtbar, lässt sich per Tastatur und Touch bedienen, skaliert ohne Rasterartefakte und bleibt vollständig offline. Canvas wird nur für das Live-Diagramm genutzt.

Die vier Bereiche AUFBAU, REGELKREIS, LIVE und TECHNISCHE DETAILS verwenden dieselbe dunkle Grundsprache und normales Scrollen. Es existieren keine Kamera-, X-Ray-, Pinning- oder Scroll-Transformationen.

## Review-Runden

### Runde 1 – Struktur und Produktlesbarkeit

Desktop, Tablet und Mobile zeigten den vollständigen Aufbau, den neuen dunklen Regelkreis und Live. Dabei wurden drei Probleme sichtbar: Der animierte SVG-Lüfter verlor durch konkurrierende Transformationen seine Einbauposition; vorhandene Komponenten erzeugten zu viele Haken; und Review-Deep-Links scrollten auf langen mobilen Seiten nicht deterministisch.

### Runde 2 – Ruhe und mechanische Glaubwürdigkeit

Die Rotation wurde auf eine innere Blattgruppe begrenzt. Vorhandene Preview-Komponenten zeigen standardmäßig keinen Marker mehr; fehlende Komponenten und der aktive Fokus bleiben sichtbar. Die Peltier-Symbole im Regelkreis wurden durch lesbares `P1 · P2` ersetzt. Der Aufbau liest sich danach ohne Labels als Becherhalter mit seitlichen Thermomodulen, PCB, Bedienung und Lüfter.

### Runde 3 – Direktkonfiguration und Responsive

Das Minimalprofil verifiziert gestrichelte Platzhalter und jeweils einen direkten `+`-Button an der realen Einbauposition. Mobile Deep-Links wurden ohne Smooth-Scroll reproduzierbar gemacht. AUFBAU, REGELKREIS und LIVE wurden anschließend in sieben Viewports geprüft: 1920×1080, 1440×900, 1366×768, 1024×768, 768×1024, 430×932 und 390×844. Alle 21 Kombinationen melden null Kollisionen und null horizontalen Overflow.

## Mindestens 30 konkrete Verbesserungen

1. Produkt-WebGL und SDF-Raymarcher entfernt.
2. Wärme-Partikel-Canvas entfernt.
3. Inline-SVG als sofort sichtbare Basis eingeführt.
4. Becher kürzer und klar konisch gestaltet.
5. Oberen Glasrand als mehrschichtigen echten Rand gezeichnet.
6. Getränk mit sichtbarer Oberfläche und ruhigem Farbverlauf ergänzt.
7. Definierten Glasboden und Reflexkante ergänzt.
8. Becher sichtbar in Haltering und Bodenaufnahme gesetzt.
9. Peltier 1 seitlich links angeordnet.
10. Peltier 2 seitlich rechts angeordnet.
11. Dünne keramische Peltierlagen statt dicker Blöcke gezeichnet.
12. Metallische Kontaktbacken zwischen Peltier und Becher ergänzt.
13. Untere Temperatur-/Kontaktplatte materiallich getrennt.
14. TMP36 #1 als schwarzes TO-92-Gehäuse mit drei Pins gezeichnet.
15. TMP36 #2 als zweites TO-92-Bauteil seitlich positioniert.
16. Dezente Sensorleitungen zur Elektronik ergänzt.
17. Bechererkennung als realen unteren Sensor dargestellt.
18. Nach außen gerichteten Lichtsensor mit Empfangsmarken ergänzt.
19. Regelmäßigen Aluminium-Lamellenkühlkörper aufgebaut.
20. Lüfterrahmen mit Einbauöffnung ergänzt.
21. Sechs echte gekrümmte Lüfterblätter statt Speichensymbol gezeichnet.
22. Langsame Rotation ausschließlich auf die Blattgruppe begrenzt.
23. PCB-Kontur mit isometrischer Perspektive und Montagebohrungen aufgebaut.
24. Leiterbahnen, Stecker und zwei Leistungsbereiche stilisiert ergänzt.
25. Pico-W-Modul mit Pads, USB-Bereich und Beschriftung erkennbar gemacht.
26. TLA2024 als eigenes IC mit Pins und Beschriftung ergänzt.
27. OLED als echtes Modul mit Rahmen, Platinenkante und zwei Werten gezeichnet.
28. MODE, DOWN, OK und UP als vier physische Taster dargestellt.
29. RGB-Ring mechanisch in den oberen Haltering integriert.
30. Fünf Status-LEDs klein auf der PCB statt als große UI-Indikatoren gezeigt.
31. 12-V-Hohlstecker seitlich am Produkt ergänzt.
32. Fehlende Baugruppen als graue gestrichelte Einbaukontur dargestellt.
33. Direkte 44-Pixel-Plusflächen an der Einbauposition eingeführt.
34. Einsetzen mit 480-ms-Opacity-/Slide-/Scale-Bewegung umgesetzt.
35. Entfernen über denselben Haken ohne Drawer ermöglicht.
36. Komponentenliste auf eine kompakte Alternative reduziert.
37. Alte Explosions-, X-Ray- und Projektionslogik entfernt.
38. Weißen Regelkreis-Hintergrund entfernt.
39. Sieben Card-Kästen durch Typografie und Linien ersetzt.
40. Rückführung auf eine einzige gerade blaue Linie reduziert.
41. Formelsymbole gegenüber normalen Begriffen visuell zurückgestuft.
42. Mobile Regelung als eigene vertikale Komposition umgesetzt.
43. Live-Ansicht auf vier Hauptwerte plus Diagramm reduziert.
44. Sekundärsignale hinter `Mehr Messwerte` verborgen.
45. Technische Daten vollständig in den eingeklappten vierten Bereich verschoben.
46. Erstes Body-Fade und Abhängigkeit von einem Renderer-Ready-Zustand entfernt.
47. Rot ausschließlich echten Fehlern vorbehalten.
48. STOPP als jederzeit verfügbaren sicheren Befehl von der PIN-Sitzung entkoppelt.

## Persona- und Einfachheitsprüfung

- Eine Person ohne Elektronikwissen erkennt Becher, Halterung, seitliche Heizmodule, Platine, Display und Lüfter.
- Schüler sehen den Wärmeweg und die wichtigsten Live-Werte ohne Fachnotation lesen zu müssen.
- Elektroniker finden Pico W, TLA2024, Treiber, Stecker, Sensorleitungen und Pins im optionalen Technikbereich.
- Regelungstechniker sehen Sollwert, Abweichung, PI, Stellgröße, Strecke, Istwert und geschlossene Rückführung korrekt.
- Produktdesigner erhalten eine ruhige Silhouette, konsistente Materialien, klare Hierarchie und keine HUD-/Neon-Überlagerung.

## Sicherheit

Read-only-Status bleibt offen. START und Sollwert benötigen ein serverseitig geprüftes, zufälliges, fünf Minuten gültiges RAM-Token. Safety bleibt nach erfolgreichem Unlock die endgültige Startinstanz. STOPP ist bewusst ohne Token erreichbar: Authentifizierung darf eine sichere Abschaltung niemals verhindern.
