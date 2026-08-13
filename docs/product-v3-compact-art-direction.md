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

## Phase 2 – Gehäusedetail, Material und Licht

Die freigegebene Form wird mit vier ruhigen Materialfamilien umgesetzt:
mattes Graphit, transparentes Glas, gedämpftes Aluminium und Amber. Ein
weiches Licht von links oben, ein sehr schwaches rechtes Fill sowie genau ein
Boden- und ein Kontaktschatten modellieren die Form. Es gibt keine weiße
Außenkontur, keine konstruktiven Linien und keine einzeln schwebenden Schatten.

Die erste Materialaufnahme
`build/ui-review/compact-v3-phase2-initial/1920x1080.png` zeigte einen zu
hellen Ring, zu metallische Kontaktstützen, eine dominante Glasoberkante und
einen zu breiten Bodenschatten. Die Freigabefassung dunkelt das Aluminium ab,
verkürzt und verschmälert beide Kontakte, senkt den Glasrandkontrast und zieht
den Schatten enger an den Fuß.

## Phase 3 – kompakte Frontbedienung

Das OLED ist 94 Einheiten breit und belegt damit 31,8 % der Produktbreite.
Eine einzige flache Einlassfläche trägt ausschließlich `32.4°` und `45.0°`.
Vier physische Taster mit 3,2 Einheiten Radius sitzen direkt darunter; ihre
Beschriftung bleibt dem Bedienkontext untergeordnet. Es gibt keinen zweiten
Panelrahmen und keine großen kreisförmigen UI-Elemente.

## Phase 4 – thermische Außenmodule

Die thermischen Zonen bleiben innerhalb der bereits vorhandenen schmalen
Kontaktformen. Zwei tonale Metallflächen und jeweils eine kurze matte
Amberfuge deuten Wärmeübertragung an, ohne Peltiers sichtbar zu machen. Drei
kleine Schlitze sitzen hinten rechts in der Basis; es gibt weder einen Rotor
noch ein Fan-Symbol, PCB, Sensoren, Kabel oder andere interne Technik.

## Visual Review 1 – Gesamtform

Referenz vor Korrektur:
`build/ui-review/compact-v3-review1-before/1920x1080.png`.

1. Der Becher wirkt noch etwas zu lang.
2. Der sichtbare Glasanteil dominiert die gesamte vertikale Komposition.
3. Der Abstand vom Becherrand zur Flüssigkeit ist leicht zu groß.
4. Der Haltering wirkt noch zu technisch präzise.
5. Die dunkle vordere Ringkante ist zu kräftig.
6. Der Kontaktschatten unter dem Becher ist zu breit.
7. Die beiden Kontaktzonen lesen sich noch als kurze Stützen.
8. Der Fuß ist optisch höher als sein geometrischer Zielwert vermuten lässt.
9. Die obere Basisfläche nimmt zu viel leere Fläche ein.
10. Halter und Frontbedienung wirken vertikal zu weit getrennt.
11. Der untere dunkle Sockelstreifen ist zu hoch.
12. Die rechte Sockelhälfte fällt etwas zu dunkel aus.
13. Der Bodenschatten ist zu breit.
14. Das Ambientlicht zieht sich zu weit nach oben und unten.
15. Die linke Glasreflexion ist zu lang.
16. Das Ambervolumen ist etwas zu deckend.
17. Der OLED-Kontrast ist im Verhältnis zum matten Fuß minimal zu hoch.
18. Die Buttonbeschriftungen konkurrieren bei großer Darstellung mit dem OLED.
19. Die Lüftung sitzt nahe an der äußeren Basisbegrenzung.
20. Die Ringellipse erscheint stärker isometrisch als die Basisellipse.
21. Der zugeschnittene Bildraum enthält oben noch zu viel Luft.
22. Das Produkt könnte bei gleicher Breite insgesamt kompakter wirken.

Korrekturen: Der Becher wird vertikal um acht Prozent verkürzt, der Fuß um 18
Prozent abgeflacht, der Bildraum enger zugeschnitten und Ambient- sowie
Bodenschatten verkleinert. Die Ringfront, Glasreflexion, Flüssigkeitsdeckung
und der Kontaktschatten werden reduziert. Dadurch rücken Becher, Halter, Basis
und Bedienung optisch zu einem einzigen niedrigen Tischprodukt zusammen.

## Visual Review 2 – Feinabstimmung

Referenz vor Korrektur:
`build/ui-review/compact-v3-review2-before/1920x1080.png`.

1. Ringfront noch etwas zu kräftig.
2. Kontaktflächen noch leicht stützenartig.
3. Flüssigkeitsrand etwas zu dick.
4. Glasrand minimal dominant.
5. Basisoberseite verbindet Halter und Bedienung noch nicht weich genug.
6. Rechte Sockelschattierung etwas zu dunkel.
7. Displayglas etwas zu schwarz.
8. Tasterlabels in der Gesamtansicht zu präsent.
9. Lüftung zu nah am rechten Rand.
10. Untere Gehäusefuge zu lang.
11. Amberfläche minimal zu gesättigt.
12. Vorderkante des inneren Halters noch zu dunkel.

Die Schlussfassung reduziert beide Ringkanten, verkürzt die sichtbaren
Kontaktflächen erneut, flacht die Flüssigkeitsellipse ab, dämpft Glas- und
Amberkontrast und hellt die rechte Basisfacette an. Displayglas und Labels
werden ruhiger; die Lüftung wandert nach innen und die untere Fuge wird um
28 Einheiten verkürzt.

## Qualitätsabnahme und Integration

Die Schlussfassung besteht die vier isolierten Lesbarkeitstests:

- 20 Prozent:
  `build/ui-review/compact-v3-quality-scale20/1920x1080.png`
- schwarze Silhouette:
  `build/ui-review/compact-v3-quality-silhouette/1920x1080.png`
- Graustufen:
  `build/ui-review/compact-v3-quality-grayscale/1920x1080.png`
- Blur:
  `build/ui-review/compact-v3-quality-blur/1920x1080.png`

Die aktive Grafik wird erst nach diesen Prüfungen aus
`ui-v7/src/product-v3-exterior.svg` in den bestehenden Generator eingebunden.
Die Außenansicht enthält keine Callout-, Fokus- oder Komponentenmetadaten. Auf
Mobile entfallen die kleinsten Tasterlabels und Lüftungsschlitze; Becher,
Halter, Basis, OLED und physische Taster bleiben erhalten.

Pflichtscreenshots:

- Produkt allein 1920 × 1080:
  `build/ui-review/compact-v3-product-final/1920x1080.png`
- Produkt allein 1440 × 900:
  `build/ui-review/compact-v3-product-final/1440x900.png`
- Aufbau-Screen 1920 × 1080:
  `build/ui-review/compact-v3-integrated-final2/1920x1080.png`
- Tablet 768 × 1024:
  `build/ui-review/compact-v3-integrated-final2/768x1024.png`
- Mobile 390 × 844:
  `build/ui-review/compact-v3-integrated-final2/390x844.png`

Before/After:

- verworfene Maschinenform:
  `build/ui-review/compact-v3-before/1920x1080.png`
- neue kompakte Außenansicht:
  `build/ui-review/compact-v3-product-final/1920x1080.png`

## Konsolidierte Änderungen

1. V6-Maschinenform als unveränderten Fallback ausgelagert.
2. neues leeres V7-Artboard angelegt.
3. neue ViewBox `150 76 340 480` definiert.
4. Becher als verbindliche Referenzbreite festgelegt.
5. Halter auf 117 % der Becherbreite begrenzt.
6. Basis auf 141 % der Becherbreite begrenzt.
7. Fuß auf 33 % der sichtbaren Becherhöhe abgeflacht.
8. Becherhöhe während der Reviews zusätzlich um acht Prozent reduziert.
9. Becher neu und leicht konisch gezeichnet.
10. klare obere Becheröffnung geschaffen.
11. eindeutigen gewölbten Becherboden ergänzt.
12. nur eine ruhige Flüssigkeitsform verwendet.
13. Flüssigkeitsellipse abgeflacht.
14. Amber-Sättigung reduziert.
15. Glastransparenz ohne Durchblick auf Technik aufgebaut.
16. Glasrandkontrast in zwei Stufen reduziert.
17. Glasreflexion verkürzt und abgedunkelt.
18. Halter als echter durchbrochener Ring statt Plattform konstruiert.
19. Ringfront verschlankt.
20. Ringmetall abgedunkelt.
21. Kontaktschatten verkleinert.
22. linke Kontaktzone verkürzt und verschmälert.
23. rechte Kontaktzone verkürzt und verschmälert.
24. thermische Metallflächen innerhalb der Kontakte gekapselt.
25. Orange auf zwei kurze matte Fugen begrenzt.
26. Basis als flache ovale Consumer-Form neu aufgebaut.
27. rechte Basisfacette aufgehellt.
28. unteren Sockelstreifen abgeflacht.
29. untere Gehäusefuge verkürzt.
30. Bodenschatten verkleinert und enger gesetzt.
31. Ambientlicht fokussiert.
32. OLED auf 31,8 % der Produktbreite begrenzt.
33. OLED ohne zusätzlichen großen Panelrahmen integriert.
34. Anzeige auf `32.4°` und `45.0°` reduziert.
35. vier Taster auf Radius 3,2 Einheiten verkleinert.
36. Tasterlabels in Kontrast und Größe reduziert.
37. Lüftung auf drei kleine rückseitige Schlitze beschränkt.
38. Lüftung vom Rand nach innen verschoben.
39. Ansichtsumschalter auf Desktop aus der Becheröffnung verlegt.
40. Desktopgröße mit seitlichem Freiraum neu kalibriert.
41. Tabletansicht mit eigenem Abstand oberhalb des Bechers versehen.
42. Mobilegrafik ohne Mikrotexte und Lüftung vereinfacht.
43. sämtliche technische Innenkomponenten aus der Außenansicht ferngehalten.
44. sämtliche Callout-/Komponenten-Metadaten aus der Außenansicht ferngehalten.
