# V3 visual qualification

The source of truth is `preview.html`, generated from `ui-v3/src/`. Screenshots are rendered
with local Chrome and Edge into the ignored `build/ui-review/` directory. Each round uses the
same status model as the Pico API; review parameters only skip the intro and choose a scenario.

## Review 1 — first-impression / WOW pass

Reference: `build/ui-review/v3-round1/1920x1080.png` plus all seven target viewports. The first
render was compared side by side with the restorable Observatory V2 screenshot at
`build/ui-review/final/1920x1080.png`.

Twenty concrete weaknesses were recorded and addressed:

1. `w(t)` competed with the giant actual-temperature number — moved right and upward.
2. The comparator touched the reading hierarchy — moved into the forward signal rail.
3. The PI node felt detached — realigned between comparator and actuator.
4. The feedback path crossed the chart data — clipped to the scene above the chart horizon.
5. The 3D assembly was too dark — increased key, rim and specular contribution.
6. Lower heat-sink detail disappeared — raised edge contrast and retained thermal under-light.
7. STOP was a small text satellite — promoted to a centred red radial action while active.
8. START and STOP visually competed during heating — inactive START is now hidden.
9. Active-state wording was ambiguous — radial state changes from START to active/STOP.
10. The intro could remain as a translucent compositor ghost in headless/reduced timing — the
    intro DOM node is removed after its bounded sequence and immediately in review mode.
11. Demo fan speed did not match the realistic reference — calibrated to about 1,850 rpm at
    68 percent heating power.
12. Product values and diagram could diverge — both now consume one normalized API model.
13. The engineering model omitted the human interface — OLED and button assembly added.
14. Status lighting was absent from the exploded story — RGB ring and LED node added.
15. Engineering labels floated without system relations — subtle ADC, I²C, PWM and tacho paths
    added.
16. The product had no robust no-WebGL story — semantic SVG assembly remains underneath.
17. Offline initially destroyed the composition — last product frame and chart are frozen under
    a restrained signal-loss veil.
18. Error could still imply heating visually — shader heat intensity is forced to zero outside
    HEATING/HOLDING.
19. The timeline looked like a conventional chart card — it is now a borderless bottom horizon.
20. The preview scenario strip omitted required faults — sensor, fan and power errors plus
    offline/reconnect are now first-class scenarios.

Outcome: V3 is clearly product-first and spatial. The classic V2 state rail, status rail and
right-side metric stack no longer define the first screen. One circular setpoint instrument,
one radial action and one optional PI detail surface are the only substantial floating controls;
there is no rectangular card grid.

## Review 2 — composition, depth, light, type, loop and chart

References: corrected 1920×1080, 1366×768 and 430×932 captures, followed by the full
`v3-round2` matrix. Sixteen additional issues were corrected:

1. The product was undersized at 1366×768 — tightened the controlled camera field of view.
2. The assembly floated in black space — added a restrained perspective ground plane.
3. Component junctions looked flat — added two-distance ambient-occlusion sampling.
4. The transparent cup wall disappeared between rim and liquid — added two subtle glass
   contour rings.
5. Glass and background had too little separation — lifted the cool material response without
   adding mirror-like reflections.
6. The `°C` unit touched the large number at mid-size desktop — increased unit spacing.
7. The state narrative was below comfortable reading size — raised its type size.
8. The cyan feedback rail dominated the scene — reduced its base opacity while retaining signal
   pulses during active control.
9. The active radial control repeated explanatory copy into the chart — suppresses that caption
   while STOP is self-explanatory.
10. Mobile WLAN label and SSID collided — changed the row to a bounded flex composition.
11. The DHCP IP clipped beyond 430 px — reserved a fixed readable mobile IP region.
12. The fourth mobile mode label overflowed — forced a four-column zero-minimum navigation grid.
13. Control-loop nodes collided with the mobile state block — moved the mobile loop sequence
    below the state/product stage.
14. The chart showed power without explicit secondary endpoints — added a quiet 100/0 percent
    axis at the right edge.
15. The preview scenario deck competed with the production composition — reduced its resting
    opacity and height on phones; focus/hover restores full emphasis.
16. Engineering exploded instantly like a debug toggle — the shader explosion uniform now eases
    toward its target, while reduced-motion switches directly.

The thermal canvas remains a separate, low-frequency visual layer. It never changes controller
state, and error/off states still force shader heat to zero immediately.

## Review 3 — professional high-end product gate

Reference matrix: `build/ui-review/v3-final-exact/`. Exact image dimensions were verified after
capture. Mode/state references:

- `build/ui-review/v3-modes-overview/1920x1080.png`
- `build/ui-review/v3-modes-thermal/1920x1080.png`
- `build/ui-review/v3-modes-engineering/1920x1080.png`
- `build/ui-review/v3-presentation-final/1920x1080.png`
- `build/ui-review/v3-modes-error/1920x1080.png`

Final gate questions:

- **Is the physical product the first read?** Yes. The largest central object is the real
  cup/plate/Peltier/fan/control stack, not a dashboard container.
- **Does depth survive without decorative blur?** Yes. Perspective, SDF geometry, glass
  construction lines, AO, fog, ground perspective, separated Z layers and restrained parallax
  establish depth.
- **Does state lighting communicate safely?** Yes. READY is cool, HEATING warm, HOLDING balanced
  green and ERROR localized red; ERROR/OFF remove thermal emission immediately.
- **Is the control loop technically readable?** Yes. Forward and feedback rails are separate,
  all four variables have live values, and the product itself occupies `G(s)`.
- **Does the timeline remain secondary?** Yes. It occupies the bottom horizon without a card
  frame and actual temperature remains visually dominant.
- **Does Engineering feel intentional?** Yes. The eased exploded view, functional annotations,
  electrical paths and focus rail read as a product teardown rather than CAD debug.
- **Does Presentation remove enough noise?** Yes. Engineering labels, process action and preview
  controls disappear; product, main values, loop and expanded history remain.
- **Is it still usable?** Yes. Keyboard focus, semantic controls, ARIA live status, minimum touch
  targets, explicit units, offline freezing and reduced motion remain.

The anti-card and anti-AI gates pass with the detailed evidence in
`docs/ui-v3-persona-review.md`. Compared with the V2 reference, V3 has no state rail, no KPI-card
stack and no lower safety-card rail in the hero. The product and its physical signal flow now
define the composition.

Known visual constraint: the embedded 3D model is a purpose-built procedural engineering model,
not an imported PCB/CAD asset. This is deliberate for flash, GPU and offline constraints; exact
component geometry can be refined later without changing the API, renderer contract or layout.
