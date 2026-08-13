# Product V3 – Compact Consumer Product

## Abgrenzung und Fallback

Die zuvor aktive Maschinenform wurde visuell verworfen und unter
`ui-v6/src/product-v3-rejected-fallback.svg` unverändert gesichert. Ihre
SHA-256-Prüfsumme am Start dieses Laufs lautet
`D70D8F393111E983FDA4C466D962EA73C0DF1C72701509F6AB0102CA21087619`.

Die neue Illustration beginnt auf einem leeren Artboard unter `ui-v7/src/`.
Keine Geometrie, Pfade, Proportionen oder ViewBox des verworfenen Produkts
werden übernommen. Firmware, Regelkreis, Live-Bereich, Chart, Auth, Safety und
Demo-Logik sind nicht Bestandteil dieses Laufs.

Before:
`build/ui-review/compact-v3-before/1920x1080.png`.

## Konstruktive Proportionen

- Becherbreite: 210 SVG-Einheiten = 100 % Referenz
- Halterbreite: 246 SVG-Einheiten = 117 %
- Basisbreite: 296 SVG-Einheiten = 141 %
- sichtbare Becherhöhe: 331 SVG-Einheiten
- Basishöhe: 124 SVG-Einheiten = 37 % der Becherhöhe in der ersten Fassung;
  nach der dritten Formkorrektur 108 Einheiten = 33 %
- Seitenkontakte: 101 SVG-Einheiten = 30 % der Becherhöhe

## Phase 1 – reine Silhouette

In dieser Phase existieren ausschließlich Becher, eine Flüssigkeitsform,
Haltering, zwei kleine Kontaktbereiche und die flache Basis. Display, Taster,
Lüftung und technische Komponenten fehlen bewusst.

Die erste Aufnahme unter
`build/ui-review/compact-v3-phase1-initial/1920x1080.png` zeigte trotz
korrekter Breitenwerte noch vier strukturelle Probleme: einen zu langen
Becher, einen abgesetzten Fuß, eine dunkle offene Plattform und zu lange
Stützen. Vor der Freigabe wurde der Becher um 41 Einheiten gekürzt, die Basis
um 37 Einheiten nach oben angebunden, beide Kontakte verkleinert und der
Halter enger auf den Becherboden gesetzt. Eine zweite Formkorrektur senkte den
Ring zusätzlich um 24 Einheiten ab und band seine verkürzten Seitenkontakte
direkt in die Oberseite des Fußes ein. Dadurch verschwand die verbliebene
Dockingstation-Lücke. Die dritte Korrektur ersetzte die schwarze Ringmitte
durch einen echten durchbrochenen Ring, reduzierte dessen Vorderkante und
flachte den Fuß von 124 auf 108 Einheiten ab.

Freigegebene Formbasis:
`build/ui-review/compact-v3-phase1-approved/1920x1080.png`.
