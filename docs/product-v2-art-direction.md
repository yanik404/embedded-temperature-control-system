# Product V2 – Art Direction

## Abgrenzung und Fallback

- Ausgangs-Branch: `feature`
- Ausgangs-Commit: `011e49cc897afdf30860648b971df76b7c9ae394`
- unveränderter Fallback: `ui-v3/src/product.svg`
- Fallback SHA-256: `16A61414E53C56D95675070C9203A597D02351C56D344B71A5F6AADFBF95BE15`
- Before: `build/ui-review/art-direction-v2-before/1920x1080.png`
- neue Außenansicht: `ui-v5/src/product-v2-exterior.svg`
- neue Reviewseite: `ui-v5/src/product-v2-review.html`
- neue ViewBox: `0 0 920 780`

Die V2 wurde auf einem leeren Artboard aufgebaut. Weder Pfade noch ViewBox,
Proportionen oder Geometriegruppen der Fallbackgrafik wurden übernommen. Die
Fallbackdatei bleibt bis zur abschließenden Qualitätsentscheidung unverändert.

## Strikte Aufbauphasen

1. **Phase A – Silhouette:** kompakte gerundete Basis, runde Aufnahme, zwei
   integrierte Seitenkörper und ein gemeinsamer Bodenschatten.
2. **Phase B – Becher:** neuer breiter, leicht konischer Getränkebecher mit einem
   Flüssigkeitskörper und einer einzelnen Randellipse.
3. **Phase C – Bedienung:** ein eingelassenes Frontpanel, kleines OLED und vier
   physische Taster.
4. **Phase D – Thermik:** symmetrische geschlossene Seitenmodule und nur eine
   Lüftungsandeutung an der rechten Gehäuseseite.
5. **Phase E – Material/Licht:** Anthrazit, Glas, Amber, dunkles Metall und
   zurückhaltendes Studiolicht ohne Einzelteil-Glows.

Bis zu diesem Punkt enthält die neue Grafik keine internen Sensoren, keine
Leiterplatte, keinen Pico, keine nackten Peltiers und keine Callouts.

## Visual Review Runde 1

Referenz: `build/ui-review/art-direction-v2-round1/1920x1080.png`

1. Der helle Aufnahmering ist das kontrastreichste Element und konkurriert mit dem Becher.
2. Die schwarze Innenöffnung ist zu groß und wirkt wie ein Loch statt wie eine Aufnahme.
3. Der Becher sitzt zwar hinter dem Ring, aber die seitliche Führung ist noch nicht klar genug.
4. Die Basis wirkt durch die mittige Spitze unten unnötig schildförmig.
5. Die linke und rechte Basisfläche sind zu symmetrisch für eine glaubwürdige 3/4-Perspektive.
6. Die rechte Seitenfläche ist nicht deutlich genug als eigene Ebene lesbar.
7. Die thermischen Seitenkörper sind zu weit nach außen gewölbt und wirken wie Griffe.
8. Die Seitenkörper enden unten relativ abrupt.
9. Orange Akzentlinien liegen zu nah an der inneren Kante und wirken aufgesetzt.
10. Die bogenförmige Fuge über dem Frontpanel erzeugt eine unnötige zweite Frontkontur.
11. Das Frontpanel ist als sehr großer schwarzer Block zu dominant.
12. Das OLED ist für die Fläche etwas zu breit.
13. Die vier Taster haben zu geringe Materialtiefe.
14. Die Tasterbeschriftungen sind in normaler Darstellungsgröße kaum lesbar.
15. Die rechte Lüftungsöffnung ist zu schmal und wirkt wie vier zufällige Striche.
16. Der Glasrand ist noch zu hell gegenüber allen Gehäuseflächen.
17. Das Getränk endet sehr gerade am unteren verdeckten Bereich und braucht weichere Überdeckung.
18. Der Becherkörper ist noch etwas zu hoch gegenüber der kompakten Basis.
19. Der leichte 3/4-Blick ist am Gehäuse schwächer als am Topring.
20. Die dunklen Gehäusematerialien verlieren rechts und unten zu viele Flächengrenzen.
21. Der Produktschatten erzeugt um die Seitenmodule einen zu breiten weichen Halo.
22. Der Bodenschatten ist etwas zu breit und flach gegenüber der Produktmasse.
23. Die Aufnahme besitzt noch keine sichtbare innere Stufe oder Kontaktauflage.
24. Das Verhältnis aus Becheröffnung und Halteröffnung ist zu großzügig.

### Maßnahmen aus Runde 1

- Ringkontrast reduzieren und Öffnung enger an den Becher führen.
- innere Aufnahmestufe und Contact Shadow ergänzen.
- Becher kürzen und tiefer einsetzen.
- rechte Gehäuseebene verbreitern, Mittelachse leicht nach links setzen.
- Basisabschluss abrunden statt zuspitzen.
- Seitenmodule flacher und stärker in die Hülle einlassen.
- Frontpanel verkleinern und besser in die Fläche integrieren.
- Taster mit Oberseite und Contact Shadow körperlicher zeichnen.
- Lüftung zu einer glaubwürdigen, eingelassenen Öffnung ausarbeiten.
- unnötige Frontfuge sowie zu starken Filter-Halo entfernen.

## Visual Review Runde 2

Referenz: `build/ui-review/art-direction-v2-round2/1920x1080.png`

1. Die Produktgrafik nutzt horizontal noch zu wenig von ihrem eigenen ViewBox und wirkt dadurch unnötig klein.
2. Der rechte Gehäuserand verschmilzt fast vollständig mit dem Hintergrund.
3. Front- und rechte Seitenfläche sind in der dunklen Tonalität nicht schnell genug unterscheidbar.
4. Die obere Aufnahme liest sich weiterhin als tiefe schwarze Öffnung statt als flache Kontaktmulde.
5. Zwischen Becher und innerer Aufnahme bleibt zu viel schwarzer Freiraum.
6. Der Becher wirkt wegen der dunklen Mulde optisch noch leicht schwebend.
7. Die Seitenmodule enden als zwei senkrechte Schenkel und erinnern weiterhin an Griffe.
8. Die Kupferlinien an den Seitenmodulen wirken dekorativ statt funktional integriert.
9. Die linke obere Gehäusefläche besitzt eine unruhige, kaum motivierte dunkle Wölbung.
10. Das Frontpanel sitzt zu tief und verliert dadurch die Beziehung zur Aufnahme.
11. OLED und Taster sind für eine schnelle Präsentationslesbarkeit noch zu klein.
12. Die vier Taster besitzen zwar Schatten, aber noch keinen klaren metallischen Rand.
13. Die Lüftung ist geometrisch verständlich, aber im rechten Dunkel zu schwach abgesetzt.
14. Der Gehäuseboden hat zu wenig sichtbare Stärke und wirkt fast wie eine dunkle Linie.
15. Der Bodenschatten ist zu schwarz und hart vom sehr dunklen Hintergrund getrennt.
16. Die Glasreflexe enden zu abrupt an der Aufnahme und unterstützen die Einsetztiefe nicht.

### Maßnahmen aus Runde 2

- ViewBox eng um die neue Silhouette legen und das Produkt im Review deutlich größer zeigen.
- Gehäuseflächen um eine Tonstufe anheben, rechte Facette und Bodensockel klar trennen.
- Aufnahme zu einer flacheren, engeren Kontaktmulde mit sichtbarer Dichtung umbauen.
- Seitenmodule über gemeinsame Flächen und kurze metallische Einleger in das Gehäuse einbinden.
- Bedienmodul etwas anheben, OLED/Taster vergrößern und Tasterränder ergänzen.
- Lüftungsrahmen und Lamellen gezielt aufhellen, ohne einen sichtbaren Außenlüfter zu erzeugen.
- weicheren Bodenschatten und zwei sparsame Kantenlichter für die Materiallesbarkeit einsetzen.

## Freigabe der Außenansicht

Referenz: `build/ui-review/art-direction-v2-exterior-final/1920x1080.png`

Die Außenansicht ist nach zwei getrennten Reviewrunden als Basis für die
Schnittansicht freigegeben. Die neue kompakte Silhouette, das breitere Glas,
die tiefe Aufnahme, das bündige Bedienfeld, die beiden integrierten
Thermikmodule und die rechte Lüftungsfläche bleiben für die Innenansicht
verbindlich. Erst ab diesem freigegebenen Stand werden technische Bauteile
innerhalb derselben Produktform ergänzt.

## Aufbauansicht

Referenz: `build/ui-review/art-direction-v2-cutaway/1920x1080.png`

Die technische Ebene verwendet dieselbe `175 72 570 666`-ViewBox, dieselbe
Silhouette, Perspektive und Becherposition. Geöffnet wird ausschließlich die
vordere Gehäusehaut. Sichtbar sind eine zentrale metallische Kontaktplatte,
zwei gegenüberliegende Heizbacken, zwei Peltiermodule, zwei TMP36-nahe
Messstellen, eine gemeinsame Elektronikplattform mit Pico, die
Strommess-Elektronik und der Lüfter hinter der rechten Außenlüftung. Kurze,
geordnete Verbindungen ersetzen frei liegende Kabel.

Der erste Schnitt-Render zeigte PCB und Lüfter noch zu stark hinter OLED und
Lüftung. Für die Freigabe wurde das Elektronikdeck angehoben, die Frontblende
als transparente Schnittabdeckung zurückgenommen und der Lüfter verkleinert
und tiefer hinter die Lamellen gesetzt.
