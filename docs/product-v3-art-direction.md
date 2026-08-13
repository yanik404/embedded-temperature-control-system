# Product V3 – Form First

## Abgrenzung

Dieser Lauf entwickelt ausschließlich eine neue, geschlossene Außenansicht.
Interne Technik, Aufbauansicht, Sensoren, Kabel, Peltier-Innenleben, Fan-Rotor,
PCB, Pico, ADC und technische Labels sind ausdrücklich nicht Bestandteil.

Die neue Illustration beginnt auf einem leeren Artboard unter `ui-v6/src/`.
Keine Pfade, ViewBoxen, Proportionen oder Geometriegruppen der bestehenden
Illustrationen werden als geometrische Basis verwendet.

Unveränderte Fallbacks am Ausgangspunkt `d50d366`:

- `ui-v3/src/product.svg`: `16A61414E53C56D95675070C9203A597D02351C56D344B71A5F6AADFBF95BE15`
- `ui-v5/src/product-v2-exterior.svg`: `D95DC60393548BCE1FFB8D6E75E4A75CBEF73FF1F0D80639759369D5C3E4030A`
- `ui-v5/src/product-v2-cutaway.svg`: `9B1C81712F5B1B6267EE980BAFF9465FDE8073B0A3E9F30722688A156FC09F31`

Before: `build/ui-review/product-v3-before/1920x1080.png`.

## Phasenvertrag

1. Silhouette: ausschließlich Becher, Aufnahme, Gehäuse und Basis.
2. Material/Licht: vier Materialien, weiches Studiolicht und ein Bodenschatten.
3. Frontpanel: kleines eingelassenes OLED und vier physische Taster.
4. Thermische Außenmodule: geschlossene, flache Seiteneinsätze und eine
   zurückhaltende Lüftungsfläche, ohne sichtbare Technik.

Erst nach der jeweiligen Screenshot-Abnahme beginnt die nächste Phase.

## Phase 1 – Silhouettenreview

Referenz: `build/ui-review/product-v3-phase1-silhouette/1920x1080.png`

1. Der Becher ist im Verhältnis zum Produkt noch zu hoch.
2. Ein klarer Becherboden ist nicht lesbar.
3. Zwischen Flüssigkeit und Glasrand liegt zu viel leere Glasfläche.
4. Die Aufnahme wirkt wie ein tiefes schwarzes Loch.
5. Der Becher scheint hinter der Aufnahme zu stehen statt tief darin zu sitzen.
6. Der Gehäusekörper ist zu hoch und zu zylindrisch.
7. Oberkörper und Basis sind nahezu gleich breit.
8. Der Fuß ist zu rund und optisch schwer.
9. Frontfläche und rechte Seitenfläche sind kaum unterscheidbar.
10. Die gewünschte leichte 3/4-Perspektive ist am Gehäuse zu schwach.
11. Der Übergang von Aufnahme zu Schulter ist zu abrupt.
12. Der Haltering ist zu breit und massiv.
13. Die innere Auflagefläche ist zu groß.
14. Der untere Abschluss ist zu weich und wenig präzise.
15. Die Gesamtform erinnert eher an einen Luftreiniger als an einen Becherhalter.
16. Für die reine Silhouettenphase sind die Verläufe bereits unnötig materialbetont.

### Korrekturen vor Freigabe

- Becher kürzen, verbreitern, absenken und mit klarer Bodenellipse schließen.
- Aufnahme verkleinern, flacher machen und enger um den Becher führen.
- Körper unterhalb der Schulter sichtbar einziehen.
- flachen, breiteren und geometrisch klareren Fuß ergänzen.
- asymmetrische Front-/Seitenflächen für eine glaubwürdige 3/4-Lesbarkeit setzen.
- Phase 1 auf wenige ruhige Flächen ohne fertige Materialeffekte reduzieren.

Die nachfolgende Fassung
`build/ui-review/product-v3-phase1-approved/1920x1080.png` wurde als reine
Formbasis für Phase 2 freigegeben.

## Phase 2 – Materialreview

Referenz: `build/ui-review/product-v3-phase2-material/1920x1080.png`

1. Der Aluminiumring ist zu hell und zieht mehr Aufmerksamkeit als der Becher.
2. Der Glasrand ist zu weiß und wirkt wie eine gezeichnete Kontur.
3. Der Kontaktschatten unter dem Ring erzeugt einen zu breiten schwarzen Halo.
4. Die Flüssigkeit wirkt trotz Verlauf noch zu flächig.
5. Die mittige Materialteilung des Gehäuses ist zu hart und technisch.
6. Die rechte Gehäuseseite fällt zu stark ins Schwarz ab.
7. Die seitlichen Sockelüberstände wirken kantig statt weich integriert.
8. Der untere Sockelabschluss ist gegen den Hintergrund kaum lesbar.
9. Die beiden Schulterkanten erinnern an Konstruktionslinien.
10. Das Ambientlicht ist zu groß und gleichmäßig verteilt.
11. Dem matten Gehäuse fehlt eine weiche Flächenmodulation.
12. Glas und Flüssigkeit besitzen noch zu ähnliche Helligkeitsverläufe.

### Korrekturen vor Materialfreigabe

- Metallkontrast und Glasrand reduzieren.
- Kontakt- und Bodenschatten enger und weicher staffeln.
- Flüssigkeit oben wärmer, unten dichter und seitlich minimal dunkler zeichnen.
- harte Gehäuseteilung durch breite, transparente Lichtflächen ersetzen.
- rechte Füllseite anheben und Sockel seitlich einziehen.
- Schulterlinien entfernen und nur zwei sehr weiche Kantenlichter behalten.

Die korrigierte Materialfassung
`build/ui-review/product-v3-phase2-approved/1920x1080.png` wurde freigegeben.
Sie bleibt auf Graphit, Glas, Aluminium und Amber beschränkt.

## Phase 3 – Frontpanelreview

Referenz: `build/ui-review/product-v3-phase3-front/1920x1080.png`

- Das Panel ist als gemeinsame, zurückgesetzte Gehäusefläche lesbar und
  schwebt nicht vor dem Produkt.
- Das OLED bleibt klein; Istwert und Sollwert sind trotzdem eindeutig.
- Die vier Taster sind gleichmäßig angeordnet und als physische Kreise lesbar.
- Für die Schlussfassung wird der Tasterkontrast leicht reduziert und die
  Panelunterkante ruhiger an den Sockel angebunden.
- Die noch sehr glatten Seiten erhalten erst in Phase 4 geschlossene,
  gehäusebündige Thermikabdeckungen.
