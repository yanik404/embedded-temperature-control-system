# V3 external design critique simulation

Seven independent professional viewpoints were applied after Review 2. Each point records the
resulting decision; this is a critique log, not a list of aspirational features.

## Automotive UX designer

1. **Critical value must be readable before controls.** Kept the actual temperature as the
   largest object and separated its unit at all desktop breakpoints.
2. **A running system needs an unmistakable stop action.** Active START disappears and the
   centred red STOP satellite takes over the radial control.
3. **Mode changes must not feel like page navigation.** All modes transform one product scene;
   tablet access to all four modes was restored.
4. **Connection loss must not blank the cockpit.** The last coherent system frame remains and
   controls lock under an explicit offline veil.
5. **Motion must encode function.** Pointer movement is only a few degrees; signal, fan, heat and
   exploded motion correspond to actual system concepts.

## Industrial designer

1. **The physical stack was initially hard to read.** Plate, Peltier, heat sink, fan and PCB now
   have distinct vertical layers.
2. **Transparent cup walls needed construction cues.** Rim, two contour rings and side rails
   define the glass without a plastic outline.
3. **The product needed a physical datum.** A dark technical ground plane anchors the assembly.
4. **Exploded labels needed functional grouping.** Thermal, sensor, power, airflow, control,
   analog and human-interface roles are named rather than assigned arbitrary part numbers.
5. **Material variety had to remain restrained.** The result uses cool glass, matte metal,
   dark PCB and one emissive thermal family; no chrome/showroom reflections were added.

## 3D artist

1. **Silhouette before detail.** The cup/plate stack was enlarged in the controlled camera.
2. **Contact depth was missing.** Two-distance shader AO was added around component junctions.
3. **The transparent shell vanished over dark liquid.** Cool contour geometry separates it.
4. **The floor could become a game grid.** Grid contrast was kept near-black and fogged with
   distance.
5. **Bloom was too easy to overuse.** The Three.js test was reduced to a subtle thresholded pass;
   the embedded renderer uses no generic postprocessing chain.

## Motion designer

1. **Intro must never delay use.** It is bounded to 1.45 seconds, skipped for review/reduced
   motion, and its DOM is removed afterward.
2. **Exploded mode snapping looked diagnostic.** The explode uniform now eases to its target.
3. **Continuous animation needs meaning.** Only fan rotation, heat rise, measurement pulse and
   loop signal movement run continuously.
4. **ERROR must override decorative continuity.** Heat output drops immediately to zero and
   red safety light becomes the only energetic state cue.
5. **Reduced motion must be a real alternative.** CSS transitions collapse, WebGL drops to a
   static 4-FPS update, particles stop and explode changes directly.

## Embedded engineer

1. **No runtime CDN on the Pico.** Production has zero external libraries and zero asset
   requests; Three.js exists only in the prototype folder.
2. **GPU work must be bounded.** Product raymarching is one draw call at capped DPR/30 FPS;
   thermal overlay is capped separately.
3. **No-WebGL devices need a coherent result.** A semantic SVG assembly is always available.
4. **Simulation must never leak into firmware.** Preview controls/data are excluded by the asset
   builder and verified in tests.
5. **The larger page must not affect lwIP ownership.** Existing asynchronous chunked sending is
   unchanged; the large-response and pbuf contracts run against the generated V3 asset.

## Control engineer

1. **The plant must be the product, not a decorative box.** `G(s)` is located at the thermal
   assembly and the route passes through the Peltier/plate/cup.
2. **Symbols and plain language must coexist.** `w(t)`, `e(t)`, `u(t)`, `y(t)` appear with
   Sollwert, Regelabweichung, Stellgröße and Istwert.
3. **Feedback direction must be unambiguous.** A separate cyan return path ends at the comparator.
4. **PI internals must remain diagnostic.** Kp, Ki, P, I, output limiting, anti-windup and period
   are read-only in the focused technical surface.
5. **Power needs a distinct chart scale.** A quiet 100/0-percent right axis was added while
   temperatures retain the dominant left scale.

## Product designer

1. **The first screen must answer what/now/next.** Product, actual value, state, target and action
   are visible without opening a detail panel.
2. **Controls should not resemble a settings form.** The selected orbital plus/minus instrument
   and radial process action replace rectangular form rows.
3. **Safety copy must stay proportional.** A thin lock signal and precise cause replace a large
   alarm card; ERROR receives stronger but localized treatment.
4. **Phone layout cannot be a scaled desktop.** It sequences product/value, state, controls,
   simplified loop and chart vertically.
5. **The experience needs a presentation story.** Fullscreen hides engineering noise while
   retaining product, actual/target, loop and timeline.

## Anti-card audit

- Visible classic rectangular metric cards in the 1920×1080 overview: **0**.
- Substantial instruments: one orbital target, one radial action, six signal nodes and a
  borderless timeline horizon.
- The PI detail surface is hidden until requested; engineering labels are leader annotations,
  not a panel grid.
- Safety uses a line/lock treatment. ERROR may add a localized dark backing for contrast.

## Anti-AI audit

- No generic KPI card grid, pill forest, rainbow gradients, large glass panel collection or
  decorative chromatic aberration.
- Small uppercase text was reduced: human-readable process labels use natural title case;
  uppercase remains for machine states, compact navigation and standards-like hardware tags.
- Orange, cyan, green and red are state/signal semantics, not arbitrary neon decoration.
- Rounded geometry is limited to physically meaningful orbital instruments and control nodes.
- Motion parallax, heat, fan and pulses all communicate state or physical flow.

## Side-by-side verdict against Observatory V2

V2 remains restorable at commit `e24098c`. It uses a strong but recognizably dashboard-like
state rail, right metric/control stack and lower safety rail. V3 removes those framing devices,
uses a genuine 3D product as the visual centre, routes the feedback loop through spatial depth,
turns the chart into a horizon and makes engineering an animated exploded state of the same
object. The comparison gate therefore passes: V3 is less dashboard-like, more spatial, more
product-specific and visually distinct while preserving the same API and safety authority.
