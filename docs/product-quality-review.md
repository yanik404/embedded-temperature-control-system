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

## Product review, round 2

The first rebuilt render at `build/ui-review/product-round1/1920x1080.png` and its missing-component state were reviewed at pixel level. Further findings and resolutions:

1. Connected hotspot checks were still visible at rest; they are now hidden until hover/focus.
2. The six relevant direct controls are enough; nonessential product hotspots were removed.
3. The hit area remains 44 px while the visible marker is only 20–24 px.
4. The component list was reduced to Sensor 1/2, Peltier 1/2, Display and Fan.
5. The initial component detail no longer forces a highlight across the product.
6. The right-side product status was shortened and reordered for scanning.
7. The mobile product occupied too little width; it now uses a controlled 122% crop.
8. Mobile cropping is contained by the stage and does not create horizontal page overflow.
9. The generated build did not depend explicitly on `product.svg`; CMake now tracks it.
10. The PI block duplicated `PI`, `REGELT` and `REGLER`; the live state is now a subordinate label.
11. The control-loop product used a generic cup glyph; it now reuses cup and side-jaw geometry.
12. The control-loop sensor used a generic reading only; it now includes a TMP36 silhouette.
13. Peltier and plant were visually separated; they are now one coupled process step.
14. The feedback label was too verbose; it now reads `RÜCKFÜHRUNG · y(t)`.
15. The target trace was solid and competed with the measured trace; it is now dashed.
16. Live header copy was redundant; the extra explanatory subtitle was removed.
17. Control lock wording was verbose; it is reduced to lock state and `ENTSPERREN`.
18. Unlocked actions now use the direct labels `SETZEN` and `START`.
19. The always-available safe `STOPP` remains visible and is not PIN-gated.
20. The PCB tray and front console now visibly connect electronics to the base.
21. The rotor contour still read as four broad petals; six separate curved blades now make the real fan construction unambiguous.

## Mechanical, thermal and sensor plausibility

- The cup is supported by a recessed lower plate and guided by two curved side uprights and an upper status ring.
- Each thin ceramic Peltier stack contacts a separate curved aluminium jaw at cup-wall height.
- The active-state heat wash follows both jaws toward the cup wall and liquid, without particles or flames.
- TMP36 #1 is clipped at the lower measuring plate; TMP36 #2 is clipped to the right side guide.
- Three leads leave each TO-92 package and route toward the electronics bay.
- The PCB rests in a visible front-left tray; the OLED and physical buttons share the front console.
- The fan is screwed to a six-fin aluminium assembly beside the right thermal stack.

## Control-loop review

The 1920×1080 loop render was reviewed for fifteen explicit criteria: station spacing, arrow alignment, direction, feedback landing, feedback direction, notation, contrast, Soll value, comparison/error, PI identification, actuator output, thermal actuator, plant identification, sensor identification and Ist value. The compact six-step layout satisfies each criterion without adding cards or a light background.

## Live review

The live render was reviewed for ten criteria: Ist prominence, Soll visibility, power visibility, fan visibility, status visibility, chart hierarchy, trace contrast, target differentiation, advanced-signal disclosure and control separation. Ist/Soll/power/status are readable within the first frame; extra measurements stay behind `Mehr Messwerte`, and commands remain in the separate authenticated control area.

## Final references

- Product, final round 2: `build/ui-review/final-product-r2/1920x1080.png`
- Product, final mobile: `build/ui-review/final-product-r2/390x844.png`
- Product, complete seven-size set: `build/ui-review/final-product/`
- Control loop, complete seven-size set: `build/ui-review/final-control-loop/`
- Live monitoring, complete seven-size set: `build/ui-review/final-live/`
- Machine-readable final 21-view audit: `build/ui-review/final-layout-audit-r2.json`

All paths are reproducible ignored build artefacts. The final audit covers 1920×1080,
1440×900, 1366×768, 1024×768, 768×1024, 430×932 and 390×844 for product, loop and live.
It reports zero visible collisions and zero horizontal overflows in all 21 combinations.

## Concrete final improvements

1. Replaced the laboratory-cylinder silhouette with a shorter, tapered drinks cup.
2. Added a separate liquid body and clearly elliptical liquid surface.
3. Reduced glass transparency and retained controlled edge highlights.
4. Added a visible rounded cup bottom.
5. Added a recessed lower cup support.
6. Added a contact shadow exactly at the support interface.
7. Added two curved structural cup guides.
8. Connected the guides to the common base.
9. Integrated the upper RGB ring into the guide structure.
10. Replaced oversized Peltier blocks with thin three-layer ceramic stacks.
11. Added a distinct dark intermediate layer to each Peltier.
12. Placed both Peltiers symmetrically at cup-wall height.
13. Added a curved aluminium contact jaw to Peltier 1.
14. Added a mirrored curved aluminium contact jaw to Peltier 2.
15. Added separate red/blue electrical leads to both Peltier stacks.
16. Mapped the active heat tint from Peltier through each jaw toward the cup.
17. Rebuilt TMP36 #1 as a clipped TO-92 package with flat face and three legs.
18. Positioned TMP36 #1 at the lower measurement plate.
19. Rebuilt TMP36 #2 as a second TO-92 package at the side contact area.
20. Added three routed leads per temperature sensor.
21. Replaced the floating PCB rectangle with a shaped project board.
22. Added a physical electronics tray connecting PCB and base.
23. Added four PCB mounting holes.
24. Added trace hints without simulating every SMD component.
25. Rebuilt Pico W as a separate turquoise elongated module.
26. Added Pico pin rows, USB shell and RP2040 area.
27. Added an identifiable TLA2024 package and pins.
28. Added two distinct current-driver areas.
29. Added connectors, capacitors, power jack and status LEDs.
30. Integrated the OLED module into the shared front console.
31. Added OLED board, bezel, glass area, pins and compact real values.
32. Replaced web-like controls with four raised physical buttons.
33. Labelled the physical buttons MODE, DOWN, OK and UP.
34. Rebuilt the sink as six regular aluminium fins with visible depth.
35. Attached the sink directly beside the thermal stack.
36. Rebuilt the fan with housing, aperture, hub and four mounting screws.
37. Replaced the rotor symbol with six separate curved blades.
38. Limited fan motion to slow rotation when the live fan is active.
39. Unified product projection, light direction and contact shadows.
40. Reduced direct configurator controls from fifteen to six essential parts.
41. Kept 44 px hit areas while reducing visible plus markers to 20–24 px.
42. Hid connected-state markers until hover/focus to calm the product.
43. Reduced the component list to six compact rows.
44. Kept one-click add and remove with the existing 480 ms insertion animation.
45. Preserved faint installation silhouettes at the real component positions.
46. Coupled Peltier and cup into one control-loop process step.
47. Reused the same side-jaw/cup visual language inside the loop.
48. Added a matching TMP36 silhouette to the sensor step.
49. Shortened and correctly landed the `RÜCKFÜHRUNG · y(t)` path.
50. Retained complete w(t), e(t), PI, u(t), actuator, G(s) and y(t) notation.
51. Removed redundant PI-state typography.
52. Made the chart target trace neutral and dashed.
53. Reduced Live to Ist, Soll, heating power, fan and status at first glance.
54. Kept secondary traces behind `Mehr Messwerte`.
55. Simplified lock copy and actions to ENTSPERREN, SETZEN, START and safe STOPP.
56. Enlarged the product with a contained crop on narrow phones.
57. Preserved a product-dominant tablet composition.
58. Verified keyboard-accessible native buttons and ARIA labels.
59. Kept production free of demo data and external asset requests.
60. Removed inactive Three.js, custom-WebGL and control-lab prototype files.
