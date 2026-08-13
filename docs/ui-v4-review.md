# V4 visual and engineering review

Review date: 13 August 2026. All captures were produced from the generated `preview.html` with
exact Chrome DevTools Protocol device metrics. Screenshots are intentionally kept in ignored build
output rather than committed as firmware assets.

## Product-first acceptance

The final 1920×1080 PRODUCT capture was evaluated against every requested question.

| Question | Result | Evidence |
|---|---|---|
| Is the product immediately the visual centre? | Pass | The complete cup, plate, Peltier, sink and controller occupy the central 60–70%. |
| Does the model read as a product rather than a dashboard graphic? | Pass | Controlled 3/4 view, studio horizon, contact darkening, metal/glass contours and restrained state light. |
| Is there enough negative space? | Pass | Copy, temperature and state use three separated edge zones; the centre remains clear. |
| Is there too much text? | Pass | PRODUCT exposes only one statement, live temperature, state and six sparse hotspots. |
| Are there too many lines? | Pass | Electrical routes are completely absent until SIGNALS or component focus. |
| Does it look like a permanent HUD? | Pass | Product mode removes the grid, most monospace labels and the control-loop geometry. |
| Does it look like a generic AI dashboard? | Pass | No card grid, metric tiles, decorative glass panels or pill collection exists. |
| Could this be a premium technical-product hero? | Pass | Product scale, quiet typography, studio lighting and asymmetric edge controls establish the hero. |
| Is temperature control still understandable? | Pass | Live temperature, state, thermal material and the focused sensor establish the function before scrolling. |

## Mode review at 1920×1080

### PRODUCT

- Answers “What is this device?” before showing engineering detail.
- Six component entry points are visible without covering the physical silhouette.
- Product, X-ray and Signals are lenses over one system, not separate pages.
- The first screen contains no traditional dashboard card.

### CONTROL / LIVE CONTROL

- The same rendered product becomes `G(s)` rather than being replaced by a block diagram.
- `w(t)`, comparison, PI, `u(t)`, Peltier, sensor `y(t)` and feedback form one closed path.
- Setpoint, Peltier power, fan RPM and STOP remain spatially separated and legible.
- The story copy occupies unused edge space and does not interrupt the loop.

### X-RAY / SIGNALS / ENGINEERING

- X-ray suppresses the opaque cup/liquid surfaces while retaining construction contours.
- SIGNALS exposes the planned electrical routes only on demand.
- ENGINEERING uses a true exploded state and shows all component hotspots, grouped focus and live values.
- The technical density is intentionally higher here; it does not leak back into PRODUCT.

### LIVE ANALYSIS

- Temperature 1 remains the dominant trace and numeric value.
- Target, temperature 2 and `u(t)` begin enabled; currents, fan and light are opt-in.
- Availability from the current API controls which signal buttons are offered.
- The chart is a transparent time plane, not an independent panel.

### GUIDED JOURNEY

- Six timed or manually navigable steps explain measurement, comparison, PI, actuator, plant and feedback.
- Each step focuses the relevant component and route while preserving the spatial loop.
- The full component information lens is suppressed during the guide to prevent duplicate narration.

### SAFETY / ERROR

- ERROR removes visible heat and forces `u(t)` to zero in the simulated status.
- Server fault title and description appear at the product rather than in a generic red alert card.
- Missing optional discovery remains neutral; only a real firmware fault receives red treatment.

## Configurator and discovery review

- The preview configuration drawer lists all 19 planned components.
- Add/remove controls mutate only a local `Set`; the module contains no `/api/` call.
- Re-adding a component runs a short assembly transition at its physical hotspot.
- The production drawer is read-only and opens the matching component focus.
- Every detail view separates **Konfiguration: vorgesehen** from its derived **Live** state.
- Buttons, individual bridges and a stopped fan explicitly avoid invented discovery claims.

## Responsive matrix

The final PRODUCT matrix is in `build/ui-review/v4-final/`:

- `1920x1080.png`
- `1440x900.png`
- `1366x768.png`
- `1024x768.png`
- `768x1024.png`
- `430x932.png`
- `390x844.png`

Desktop reserves the most space for studio rendering. Tablet preserves the complete product and DHCP
identity. Mobile changes composition: product occupies the upper stage; measurement, state and entry
controls move below it; only the primary sensor hotspot remains in quiet PRODUCT mode. X-ray,
Signals and Engineering can intentionally expose more hardware targets.

Additional final captures:

- `build/ui-review/v4-final-control/1920x1080.png`
- `build/ui-review/v4-final-analysis/1920x1080.png`
- `build/ui-review/v4-final-engineering/1920x1080.png`
- `build/ui-review/v4-final-xray/1920x1080.png`
- `build/ui-review/v4-final-guide/1920x1080.png`
- `build/ui-review/v4-final-error/1920x1080.png`
- `build/ui-review/v4-final-edge/1440x900.png`

Chrome rendered the complete matrix. Microsoft Edge rendered the independent 1440×900 compatibility
capture. Native Safari is not available on the Windows build host; the implementation remains within
WebGL 1, Canvas 2D, SVG, Fetch, IntersectionObserver and standards-based CSS, with the existing SVG
fallback for unavailable WebGL.

## Refinement review rounds

### Round 1 — Product (20 findings resolved)

The final `v4-refine-product` and seven-viewport `v4-refine-final` captures were checked for product
scale, silhouette, negative space, text hierarchy, hotspot attachment, lighting, first-screen scope,
header weight, technical detail leakage, thermal color, state prominence, temperature prominence,
configurator discoverability, lens separation, card count, monospaced text, connection prominence,
touch targets, focus clarity and preview chrome. The product now occupies the visual centre while
control, chart, safety detail and Engineering remain outside the first screen.

### Round 2 — Control loop (15 findings resolved)

The `v4-refine-control` capture was checked for setpoint, comparator, error, PI, output, explicit
Peltier actuator, product-as-plant, TMP36 sensor, feedback return, arrow direction, value legibility,
state consistency, control separation, safety authority and ambient animation. The product remains
the plant and `u(t)` now names the Peltier function directly.

### Round 3 — Engineering and full scroll (15 findings resolved)

The `v4-refine-engineering`, `v4-refine-safety` and `v4-refine-analysis` captures were checked for
exploded geometry, attached hotspots, planned/live separation, unknown status, disconnected status,
real faults, pin detail, optional hardware, configuration grouping, neutral colour, IP visibility,
Safety/System separation, chart availability, story order and sticky continuity. A scripted Chrome
audit traversed all six chapters: the stage remained at viewport bounds, the story advanced through
Configure, Loop, Live Control, Analysis, Safety and Engineering, and no horizontal overflow occurred.

## Corrections made during review

1. Reduced the permanent cyan/HUD treatment in PRODUCT.
2. Removed the product-mode floor grid while preserving it as an Engineering cue.
3. Enlarged the physical model relative to interface chrome.
4. Limited first-screen hotspots to important entry points.
5. Reduced mobile PRODUCT to one meaningful sensor hotspot.
6. Added separate Product/X-ray/Signals lenses.
7. Made X-ray remove opaque vessel/liquid surfaces.
8. Increased Engineering signal-route contrast only in technical modes.
9. Dimmed unrelated hotspots during component focus.
10. Added a real camera-like focus translation and scale transition.
11. Removed the state readout behind an open component lens.
12. Prevented inactive safety content from appearing during normal analysis.
13. Moved analysis narration away from the actual-temperature hierarchy.
14. Suppressed duplicate component detail during the guided journey.
15. Preserved DHCP IP visibility at tablet width.
16. Converted the chart legend into direct signal controls.
17. Hid unavailable optional signals rather than drawing invented values.
18. Added truthful neutral states for non-discoverable hardware.
19. Added preview-only component assembly interaction.
20. Added exact query-addressable review states for deterministic screenshots.
21. Separated planned configuration from preview-only physical connection state.
22. Added full, minimal, partial and sensorless hardware profiles.
23. Made missing API booleans an explicit unknown state instead of implicit `false`.
24. Removed static claims such as a fictitious monitored 3.3-V logic status.
25. Added a dedicated Safety matrix for temperature, current, fan, cup and power checks.
26. Marked WLAN, webserver and sensor bus as system context rather than thermal safety checks.
27. Replaced the full-screen offline veil with a compact edge status while preserving the model.
28. Grouped the configurator into temperature, actuators, sensors, interface and system.
29. Increased hotspot hit targets to 44 px while retaining visually small markers.
30. Added exact German ARIA labels ending in “öffnen” and a visible keyboard focus ring.
31. Added ESC and free-stage click return to the overview camera.
32. Coupled hotspot translation to the component-focus camera movement.
33. Simplified the header and hid DHCP details outside Engineering.
34. Increased Product typography sizes and reduced micro-label density.
35. Enlarged the Product scene to roughly 72% of the central stage width.
36. Clarified `u(t)` as the Peltier actuator at the control-loop node.
37. Corrected chart time labels for seconds, minute:second and minutes.
38. Prevented absent target, power, fan or optional sensors from becoming zero-lines.
39. Removed obsolete Engineering text tags duplicated by interactive component details.
40. Consolidated old mobile override patches and reduced `!important` usage from 75 to zero.
41. Kept START, STOP and setpoint behavior on the established API path without changes.
42. Kept PreviewDriver entirely out of the generated Pico production document.
43. Kept the single-file production dashboard below its existing 128-KiB budget.
44. Added a realistic regression contract for a missing `temp2_ok` field.
45. Re-rendered Product at all seven required exact CSS viewports.
46. Re-rendered Control, Analysis, Safety and Engineering as deterministic review states.
