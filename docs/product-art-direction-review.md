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
