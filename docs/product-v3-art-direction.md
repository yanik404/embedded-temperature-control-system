# Product V3 – Form First

## Abgrenzung

Dieser Lauf entwickelt ausschließlich eine neue, geschlossene Außenansicht.
Interne Technik, Aufbauansicht, Sensoren, Kabel, Peltier-Innenleben, Fan-Rotor,
PCB, Pico, ADC und technische Labels sind ausdrücklich nicht Bestandteil.

Diese inzwischen verworfene Illustration begann auf einem leeren Artboard
unter `ui-v6/src/` und liegt heute als unveränderter Fallback unter
`ui-v6/src/product-v3-rejected-fallback.svg`.
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

## Phase 4 – Außenmodulreview

Referenz: `build/ui-review/product-v3-phase4-exterior/1920x1080.png`

- Die geschlossenen Abdeckungen zeigen die thermischen Zonen ohne nackte
  Peltierplatten oder Innenleben.
- Ihre erste Fassung ist noch zu lang und durch den hellen Mittelstreifen zu
  griffähnlich.
- Die Abdeckungen werden flacher, kürzer und tonaler näher an das Gehäuse
  gelegt; Orange bleibt nur eine sehr kurze, matte Fuge.
- Die Lüftungsfläche sitzt noch zu tief und wie ein separates Modul. Sie wird
  kleiner und bündig in die rechte Gehäusefacette verschoben.
- OLED-Texte und Taster bleiben unverändert ruhig; es werden keine weiteren
  technischen Details ergänzt.

Die korrigierte Schlussfassung liegt unter
`build/ui-review/product-v3-exterior-final/1920x1080.png`. Sie zeigt nur
geschlossene Außenflächen; weder Rotor noch Peltierplatten, Sensoren, PCB,
Pico, ADC oder Kabel sind sichtbar.

## No-UI-Qualitätsfreigabe

- 20 Prozent: `build/ui-review/product-v3-quality-scale20/1920x1080.png`
- schwarze Silhouette: `build/ui-review/product-v3-quality-silhouette/1920x1080.png`
- Blur: `build/ui-review/product-v3-quality-blur/1920x1080.png`
- Graustufen: `build/ui-review/product-v3-quality-grayscale/1920x1080.png`

Alle vier Darstellungen erhalten die Hierarchie Becher, Aufnahme, Halter und
flacher Fuß. Der Fünf-Sekunden-Befund lautet „kompakter intelligenter
Becherhalter“; Display, Lüftung und seitliche Thermikzonen bleiben als
untergeordnete Produktdetails lesbar.

## Integration und responsive Schlussabnahme

Die damalige Außenansicht wurde erst nach Abschluss der vier Formphasen aus
der heute als `ui-v6/src/product-v3-rejected-fallback.svg` erhaltenen Datei in
die bestehende Seite eingebettet. Die
V2-Außenansicht bleibt bytegenau als Fallback erhalten; die vorhandene
technische Bestandsansicht bleibt funktional, wurde in diesem Lauf aber weder
neu gezeichnet noch erweitert. Die neue Außenansicht enthält keine
`data-part`-, `data-focus`- oder `data-callout-anchor`-Metadaten.

Pflichtabnahmen:

- Produkt allein 1920 × 1080:
  `build/ui-review/product-v3-product-only-final/1920x1080.png`
- Produkt allein 1440 × 900:
  `build/ui-review/product-v3-product-only-final/1440x900.png`
- Produkt im Aufbau-Screen 1920 × 1080:
  `build/ui-review/product-v3-integrated-final/1920x1080.png`
- Tablet 768 × 1024:
  `build/ui-review/product-v3-responsive-final/768x1024.png`
- Mobile 390 × 844:
  `build/ui-review/product-v3-responsive-final/390x844.png`

Visual Review Runde 1 umfasst die eigenständige Silhouetten- und
Materialkritik mit 16 beziehungsweise 12 Befunden. Visual Review Runde 2
umfasst Frontpanel, geschlossene Thermikmodule und die responsive Einbettung.
Gegenüber dem Before-Screenshot ist die neue Form kürzer, breiter, geschlossener
und deutlich produktartiger; die dominante technische Querstruktur, offene
Seitenteile und freigestellten Baugruppen des Fallbacks sind entfallen.

### Konsolidierte Verbesserungen

1. Becherhöhe reduziert.
2. Becherbreite erhöht.
3. konische Glasform neu proportioniert.
4. Becherboden eindeutig geschlossen.
5. Glas tiefer in die Aufnahme gesetzt.
6. leere Glaszone über der Flüssigkeit verkleinert.
7. ruhige einzelne Flüssigkeitsform geschaffen.
8. Flüssigkeitsoberfläche als klare Ellipse ausgebildet.
9. Aufnahme enger um den Becher geführt.
10. Aufnahme von einer Vertiefung zu einem räumlichen Haltering entwickelt.
11. Haltering flacher gestaltet.
12. Halteringbreite reduziert.
13. Gehäusehöhe reduziert.
14. Gehäusemitte sichtbar eingezogen.
15. Front- und Seitenfacette getrennt lesbar gemacht.
16. leichte 3/4-Perspektive verstärkt.
17. Schulterübergang weicher an die Aufnahme angebunden.
18. Basis verbreitert.
19. Basis abgeflacht.
20. Basisüberstände abgerundet und eingezogen.
21. unteren Abschluss präziser und ruhiger gestaltet.
22. Aluminiumkontrast des Rings reduziert.
23. helle Glasoutline zurückgenommen.
24. Kontakt- und Bodenschatten enger gestaffelt.
25. Flüssigkeit oben wärmer und unten dichter modelliert.
26. harte mittige Materialteilung durch Flächenlicht ersetzt.
27. rechte Gehäuseseite mit weichem Fill aufgehellt.
28. technische Schulterlinien entfernt.
29. Ambientlicht verkleinert und fokussiert.
30. matte Graphitfläche subtil moduliert.
31. OLED und Taster in eine gemeinsame Vertiefung integriert.
32. Tasterkontrast reduziert und physische Abstände vereinheitlicht.
33. Seitenabdeckungen verkürzt und gehäusebündig gemacht.
34. orange Akzente auf kurze matte Fugen begrenzt.
35. Lüftungsfläche verkleinert und in die rechte Facette integriert.
36. neue ViewBox eng auf Produkt und notwendigen Schatten zugeschnitten.
37. Produkt in der Desktop-Bühne vergrößert und 24 px höher gesetzt.
38. Tablet- und Mobile-Skalierung ohne Überlauf oder Beschnitt freigegeben.
