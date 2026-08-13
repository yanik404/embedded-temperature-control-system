# Product quality review

## Preserved baseline

- Branch: `feature`
- Stable source commit before this quality pass: `d29a6e55de549aaf733330dd6208564b62290b58`
- Product reference: `build/ui-review/baseline-d29a6e5-product/1920x1080.png`
- Control-loop reference: `build/ui-review/baseline-d29a6e5-control-loop/1920x1080.png`
- Live reference: `build/ui-review/baseline-d29a6e5-live/1920x1080.png`
- Baseline UI build: preview 84,311 B; production 70,712 B; theoretical gzip 20,814 B.

The generated screenshots remain build artefacts and are intentionally ignored by Git. This document records the reproducible paths and the exact source commit.

## Product review, round 1

The 1920×1080 baseline was reviewed before editing. The concrete findings are:

1. The cup is too tall and cylindrical for a normal drinks cup.
2. The outer rim is oversized and reads as laboratory glassware.
3. The liquid has no convincing free surface and merges with the wall.
4. The cup bottom is visually lost inside the base.
5. The cup appears to float above a dark aperture.
6. The upper holder ring is not mechanically connected to the base.
7. The left and right guides are too far behind the cup to explain support.
8. The bottom support looks like an isolated metal tile.
9. Both Peltier assemblies are much too thick.
10. Ceramic, semiconductor layer and contact jaw are not clearly separated.
11. The contact jaws are flat slabs rather than cup-conforming surfaces.
12. The heat path is only suggested by detached orange arcs.
13. TMP36 #1 is too large and has no visible mounting clip.
14. TMP36 #2 floats in front of the right contact assembly.
15. Sensor leads do not visibly terminate at a connector.
16. The PCB is a plain skewed rectangle placed in front of the device.
17. The PCB has no mechanical tray or attachment to the prototype.
18. The Pico module is too abstract and its USB connector is unclear.
19. The TLA2024 and current paths are hard to identify spatially.
20. The OLED is an independent panel rather than part of a console.
21. The four controls read as tiny interface marks instead of physical buttons.
22. The fan reads as a flat turbine symbol and has no mounting screws.
23. The heatsink is represented mainly by lines, without fin depth.
24. Product components use incompatible front and isometric projections.
25. Contact shadows do not explain which parts touch each other.
26. The RGB ring is detached from a tangible upper holder structure.
27. Large connected-state check buttons cover the product unnecessarily.
28. The right-hand component list is too long and dashboard-like.
29. The headline and explanation compete with the product for first-frame space.
30. The illustration occupies less of the available main-column width than intended.

These findings define the scope of the visual rebuild. Firmware, HTTP API, authentication and safety semantics are explicitly outside this quality pass.
