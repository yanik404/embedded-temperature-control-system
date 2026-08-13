# Produktillustration – Art-Direction-Review

## Unveränderte Ausgangsbasis

- Branch: `feature`
- Baseline-Commit: `4c2e31ed5ae4e34b42fe4ef5b3c3d70486b9f22a`
- Quellgrafik: `ui-v3/src/product.svg`
- Referenzaufnahme: `build/ui-review/product-redesign-baseline/1920x1080.png`
- Aufnahme: 1920 × 1080, Szenario `heating`, Fokus `product`

Die Ausgangsbasis zeigt alle Baugruppen gleichzeitig. Dadurch konkurrieren Becher,
offene Peltiermodule, Leiterplatte, Frontkonsole und ein seitlich freistehender
Lüfter um Aufmerksamkeit. Dieser Commit dient ausschließlich als reproduzierbarer
Vorher-Punkt; Firmware, API und Sicherheitslogik bleiben unberührt.

Die drei neuen Art-Direction-Varianten und die begründete Auswahl werden nach den
vergleichenden Renderings in diesem Dokument ergänzt.

## Vergleich A / B / C

Alle Aufnahmen verwenden 1920 × 1080, `heating`, 32,4 °C Istwert, 45,0 °C
Sollwert, 68 % Heizleistung und dieselbe Komponentenliste.

| Variante | Aufnahme | Produkt | sofort verständlich | hochwertig | ruhig | Technik | Passung |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| A · Premium 3/4 | `build/ui-review/product-variant-a/1920x1080.png` | 5/5 | 5/5 | 5/5 | 5/5 | 3/5 | 5/5 |
| B · 2.5D-Schnitt | `build/ui-review/product-variant-b/1920x1080.png` | 3/5 | 4/5 | 3/5 | 3/5 | 5/5 | 4/5 |
| C · offene Basis | `build/ui-review/product-variant-c/1920x1080.png` | 4/5 | 5/5 | 4/5 | 4/5 | 4/5 | 4/5 |

### Variante A

Die Außenhülle, der integrierte Haltering und der kompakte Sockel erzeugen die
stärkste Silhouette. OLED, Taster, seitliche Heizabdeckungen und der in die rechte
Sockelfläche integrierte Lüfter lesen sich als Teile desselben Produkts. Es gibt
keine Kabel und keine offen aufgesetzte Leiterplatte.

### Variante B

Die kontrollierte Schnittansicht legt beide Ketten `Becher ↔ Aluminiumkontakt ↔
Peltier` frei. TMP36 #1, TMP36 #2, PCB/Pico und das gemeinsame Lüfter-/Kühlkörpermodul
sind mechanisch zuordenbar, ohne zu einer Explosionszeichnung auseinanderzufallen.

### Variante C

Die obere Produkthälfte bleibt geschlossen, die Servicebasis ist geöffnet. Das
ist ein brauchbarer Kompromiss, wirkt als Standardansicht aber weniger fertig als
A und erklärt die thermische Kopplung nicht so direkt wie B.

## Auswahl

**A wird die Standard-Außenansicht. B wird die einzige alternative Ansicht
`AUFBAU`.** C bleibt über den Commit-Verlauf und den obigen Screenshot
nachvollziehbar, wird aber vor dem finalen Produktionsasset entfernt. So erfüllt
der erste Blick die Produktwirkung; technische Details erscheinen erst bewusst
durch `AUFBAU`, Hover, Tastaturfokus oder Klick.

## Review der Ausgangsgrafik: 30 konkrete Probleme

1. Becher, Basis, PCB und Lüfter bildeten vier konkurrierende Silhouetten.
2. Das PCB lag sichtbar vor der Basis und wirkte nachträglich angeklebt.
3. Der Lüfter stand als separater großer Kasten rechts neben dem Produkt.
4. Beide weißen Peltierplatten waren in der Normalansicht vollständig freigelegt.
5. Vierfarbige Lüfterleitungen endeten sichtbar außerhalb des Geräts.
6. Drei Sensorleitungen verliefen dauerhaft quer durch die Produktzeichnung.
7. Glas, Getränkekörper und Innenellipse lasen sich wie zwei ineinander gestellte Becher.
8. Der Becher war relativ schmal und hoch und wirkte eher wie ein Messzylinder.
9. Die Getränkefläche hatte zu viele eigene Außenkonturen.
10. Zu viel Glastransparenz legte gleichzeitig Halterung, Sensoren und Boden frei.
11. Offene rückwärtige Streben verhinderten eine geschlossene Gehäusewirkung.
12. Die Aufnahme bestand aus sichtbaren Einzelstreben statt aus einer Hülle.
13. Viele Bauteile hatten eine eigene helle Kontur im CAD-/Comic-Stil.
14. Aluminium, Keramik und Kunststoff wurden stärker durch Linien als Flächen getrennt.
15. Mehrere Drop-Shadows ließen Teile voneinander abheben statt sie zu verbinden.
16. Der RGB-Ring schwebte optisch vor der mechanischen Aufnahme.
17. Die seitlichen Kontaktbacken waren mechanisch nicht eindeutig abgedeckt.
18. Die thermische Kette Kontaktbacke ↔ Peltier war nur bei genauer Betrachtung lesbar.
19. Der Kühlkörper war vom Lüfter als eigenes Objekt getrennt.
20. Das OLED wirkte wie ein separates Modul auf einer schrägen Zusatzplatte.
21. Die vier Taster hatten durch die Perspektive keine ruhige gemeinsame Bedienleiste.
22. Winzige PCB-Traces, Pins und Beschriftungen erzeugten visuelles Rauschen.
23. TMP36 #1 dominierte den Becherboden in der Standardansicht.
24. TMP36 #2 stand offen im Heizbereich und konkurrierte mit dem Becher.
25. Der Sockel hatte keine einfache, starke Vorder-/Seitenflächenhierarchie.
26. Vorderpanel, PCB und Lüfter verwendeten unterschiedliche Einbauperspektiven.
27. Die Wärmefarbe lag großflächig als Glow hinter mehreren Bauteilen.
28. Die Komponentenliste nutzte auf Desktop nur eine lange Spalte.
29. Historische Mehrlinienmodi konnten mehrere technische Routen gleichzeitig zeigen.
30. Der Regelkreis gab Vergleich und Sensor dasselbe Gewicht wie den fünf Hauptpunkten.

## Umgesetzte visuelle Änderungen: 55 konkrete Punkte

1. Drei eigenständige 2.5D-Art-Directions A, B und C wurden gebaut.
2. Alle Varianten wurden mit identischen Daten und 1920 × 1080 gerendert.
3. Variante A wurde anhand von sechs festen Kriterien als Standard gewählt.
4. Variante B wurde zur kontrollierten technischen Aufbauansicht weiterentwickelt.
5. Variante C wurde nach dem Vergleich aus dem Produktionsasset entfernt.
6. Ein einziger Umschalter `AUSSENANSICHT | AUFBAU` ersetzt technische Modi.
7. Die Außenansicht ist der garantierte Startzustand.
8. Eine zusammenhängende anthrazitfarbene Außenhülle verbindet Aufnahme und Basis.
9. Die Basis hat jetzt eine klare 3/4-Geometrie mit Front- und rechter Seitenfläche.
10. Ein gemeinsamer Bodenschatten verankert das gesamte Produkt.
11. Der Becher wurde breiter und weniger hoch proportioniert.
12. Der Becher besitzt nur noch eine dominante Außenkontur.
13. Rand und Boden wurden klar, aber zurückhaltend definiert.
14. Eine einzelne Reflexionskante ersetzt mehrere helle Glaskonturen.
15. Das Getränk ist eine Fläche innerhalb des Glases statt ein zweites Gefäß.
16. Die Amber-Flüssigkeit besitzt eine klare obere Ellipse.
17. Die Flüssigkeitstransparenz wurde reduziert und beruhigt.
18. Der obere Haltering wurde fest in die Gehäusesilhouette integriert.
19. Der RGB-Ring sitzt als dezente Punktfolge im Haltering.
20. Der RGB-Ring übernimmt weiterhin die echten Zustandsfarben.
21. Linkes und rechtes Heizmodul sind außen dunkle Gehäuseabdeckungen.
22. Die Heizmodule sind symmetrisch und an die Becheraufnahme angepasst.
23. Wärme wird außen nur als schmale orange Akzentkante gezeigt.
24. Es gibt keine Wärme-Partikel oder großflächigen Neon-Glows.
25. Im Aufbau werden beide keramischen Peltiers als dünne Schichten sichtbar.
26. Gebogene Aluminium-Kontaktbacken folgen der Becherkontur.
27. Die Kette Becher ↔ Aluminium ↔ Peltier ist links und rechts lesbar.
28. TMP36 #1 wird als kleines TO-92-Bauteil am unteren Kontakt dargestellt.
29. TMP36 #2 wird als kleines TO-92-Bauteil am Seitenkontakt dargestellt.
30. Beide Sensoren bleiben in der Außenansicht unsichtbar, außer bei Fokus.
31. Sensorleitungen sind in der normalen Ansicht vollständig entfernt.
32. Die Aufbauansicht zeigt die Sensororte ohne Kabelwolke.
33. Das PCB wurde in einen kontrollierten Ausschnitt der Elektronikbasis gelegt.
34. Der Pico W besitzt erkennbare Platinenform, USB-Bereich und Pinpunkte.
35. Strommessung und Treiber sitzen auf derselben Hauptplatine.
36. Das Frontpanel ist eine einzige integrierte Fläche.
37. Das OLED liegt bündig im Frontpanel statt als aufgeklebtes Modul.
38. Vier Taster sind gleich groß, gleich tief und gleich ausgerichtet.
39. Das Aufbau-Frontpanel wurde seitlich verkürzt, damit PCB/Pico sichtbar bleibt.
40. Lüfter und Kühlrippen bilden ein gemeinsames thermisches Modul.
41. Der Lüfter besitzt Rahmen, Nabe und sechs gebogene Blätter.
42. Das Kühlmodul sitzt innerhalb der rechten Basisfläche statt daneben.
43. Sichtbare Lüfterkabel wurden vollständig entfernt.
44. Helle Konturen wurden auf Glasrand, Keramik und notwendige Metallkanten begrenzt.
45. Materialien werden primär über Flächen, Verläufe und dezente Highlights getrennt.
46. Die rechte Komponentenübersicht nutzt auf Desktop zwei kompakte Spalten.
47. Alle zwölf Zuordnungen passen bei 1080p ohne internes Scrollen.
48. Im Standardzustand ist keine Callout-Linie sichtbar.
49. Hover, Tastaturfokus oder Klick zeigt maximal eine direkte Linie.
50. Die frühere große Umwegroute für linke Komponenten wurde entfernt.
51. Inline-Details wurden auf Typ, Einbauort und Live-Wert reduziert.
52. GPIO, ADC und Busdaten bleiben im Bereich Technische Details.
53. Mobile blendet Callout-Linien vollständig aus.
54. Der Regelkreis wurde auf SOLL → PI → HEIZEN → BECHER → IST reduziert.
55. Vergleich, Sensor und Rückführung sind visuell untergeordnet; Live bleibt bei einer großen Temperatur, drei Sekundärwerten und dem Chart.

## Qualitätsprüfungen und finale Aufnahmen

- Außenansicht ohne UI: `build/ui-review/product-only/1920x1080.png`
- Außenansicht bei 20 %: `build/ui-review/product-scale20/1920x1080.png`
- Außenansicht in Graustufen: `build/ui-review/product-grayscale/1920x1080.png`
- reine Silhouette: `build/ui-review/product-silhouette/1920x1080.png`
- finaler Aufbau ohne UI: `build/ui-review/build-only-final/1920x1080.png`
- finaler erster Screen: `build/ui-review/selected-exterior/1920x1080.png`
- finaler Responsive-Satz: `build/ui-review/final-product/{1920x1080,1440x900,1366x768,1024x768,768x1024,430x932,390x844}.png`
- Fokuszuordnung: `build/ui-review/selected-hover-direct/1920x1080.png`
- Regelkreis: `build/ui-review/review-loop/1920x1080.png`
- Live: `build/ui-review/review-live/1920x1080.png`

Der 20-%-Test erhält die Silhouette aus Becher, Seitenteilen und Basis. Der
Graustufentest trennt Glas, Getränk, Gehäuse, Bedienpanel und Lüfter weiterhin.
Der Silhouettentest liest als Becherhalter und nicht als lose Maschine. In der
Aufbauansicht sind beide thermischen Ketten, beide Sensororte, die integrierte
Elektronikbasis und das gemeinsame Kühlmodul mechanisch plausibel.

Der Layout-Audit unter `build/ui-review/final-layout-audit.json` prüfte Produkt,
Regelkreis und Live bei 1920×1080, 1440×900, 1366×768, 1024×768, 768×1024,
430×932 und 390×844: **21/21 Kombinationen ohne Kollision und ohne horizontalen
Overflow**.

## Before / After und Performance

| Merkmal | Vorher | Nachher |
| --- | --- | --- |
| Standardprodukt | offene Baugruppen | geschlossene Premium-Außenansicht |
| Innenleben | dauerhaft sichtbar | nur in Aufbau oder Fokus |
| Kabel | Sensor- und Lüfterleitungen sichtbar | im Standard vollständig entfernt |
| Callouts | Mehrlinienvarianten vorhanden | keine Linie, bei Fokus genau eine |
| PCB | vor der Basis | kontrollierter Ausschnitt in der Basis |
| Lüfter | separater Kasten | integriertes Lüfter-/Kühlkörpermodul |
| Regelkreis | sechs gleichgewichtige Stufen | fünf Hauptpunkte, zwei kleine Zwischenelemente |
| Produktion | 86.813 Byte | 87.431 Byte |
| externe Assets | 0 | 0 |
| WebGL | 0 Draw Calls | 0 Draw Calls |

Die Produktion bleibt ein einziges Inline-SVG-/DOM-/Canvas-Dokument und liegt
unter dem 128-kB-Vertrag. API, Authentifizierung, Regler, Safety und Firmwarelogik
wurden für diesen visuellen Lauf nicht geändert.

## Regressionsergebnis

- deterministischer UI-Build: bestanden
- Dashboard-/Produkt-/Digital-Twin-Verträge: bestanden
- echter Browser: Außen/Aufbau, Fokus, Add/Remove, Tastatur und Mobile bestanden
- große HTTP-Antwort: 87.431 Byte in 60 Chunks bei `TCP_SND_BUF=29.200` bestanden
- lwIP-pbuf-Ownership: bestanden
- Web-Authentifizierung: bestanden
- Controller-Hosttest: bestanden
- Status-LED-Hosttest: bestanden
- vollständiger Pico-W-Build: bestanden
- UF2: `build/vscode/temperature_control.uf2`
