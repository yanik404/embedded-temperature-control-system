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

