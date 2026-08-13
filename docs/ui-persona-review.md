# Persona and anti-generic-design review

Review basis: the second seven-viewport render of the real local build, including READY,
HEATING, HOLDING, ERROR and DISCONNECT states. Comments are intentionally critical. A
check mark means the point influenced revision two or the final polish; a dash means it was
considered but deliberately rejected.

## 1. Electrical-engineering lecturer

1. ✓ The process illustration must not obscure that `y(t)` is a measured signal, not a
   calculated cup color. The TMP36 probe and live value remain visibly wired together.
2. ✓ The forward and feedback directions need different signal semantics. Orange/state
   energy and cyan sensor feedback use separate paths and timing.
3. ✓ The error should use kelvin for a temperature difference. Hero and loop use `K`, while
   absolute sensor and target values retain `°C`.
4. ✓ The PI equation alone is insufficient for assessment. Kp, Ki, P, I, anti-windup,
   clamping and cycle time remain available in the read-only engineering layer.
5. ✓ A graph should not open empty in a presentation. The local preview now preloads a
   realistic five-minute trace and continues it live.

## 2. UI/UX designer

1. ✓ A numeric input with browser-native spinner visually fights the rotary control. It was
   replaced by a locale-aware decimal text field with explicit plus/minus controls.
2. ✓ The phone header must never trade live information for horizontal overflow. Its tracks
   now have bounded intrinsic widths and clipped network values.
3. ✓ Small annotations outside the product drawing became visual debris on phones. Repeated
   labels are hidden there while the integrated values remain in the process narrative.
4. ✓ The scenario console is a developer aid and must not enter review/presentation
   composition. Deterministic review mode now removes it completely.
5. – A conventional bottom navigation would shorten travel on phones, but would turn the
   piece back into a standard app shell. Native scrolling and progressive disclosure stay.

## 3. Embedded engineer

1. ✓ Preview-only data and controls must not consume Pico flash. The production build is
   tested to exclude the simulation driver and scenario markup.
2. ✓ A GPU animation must stop when reduced motion is requested. Low-Motion now cancels the
   recurring animation frame and draws only on state changes.
3. ✓ No runtime font, script or image request may depend on Internet access. Both builds
   have zero external asset URLs; production is a single HTTP body.
4. ✓ The much larger page must preserve lwIP ownership and asynchronous TCP chunking. Both
   existing contract tests run against the generated 89 KB production asset.
5. ✓ Generated C should never be hand-edited. CMake and CI now rebuild and byte-compare the
   readable sources, production page and `web_assets.h`.

## 4. Normal user

1. ✓ The current state must be understandable before reading a diagram. State color, state
   spine and the plain German state description always appear above the control.
2. ✓ STOP must remain available whenever the Pico is live, even when START is locked. Its
   command remains independent and visually dominant in active heating states.
3. ✓ A disconnected page should explain itself without erasing useful history. Controls
   lock, the connection copy changes, and the last chart remains behind an offline layer.
4. ✓ Decimal input should accept the comma users type in German. Parsing accepts comma or
   point, normalizes the range and sends the server a decimal point.
5. – Hiding engineering terminology entirely would be simpler, but it conflicts with the
   project’s teaching goal. Terms stay paired with plain-language descriptions.

## 5. Product designer

1. ✓ The physical product must be the identity, not decorative wallpaper. The unique
   thermal cutaway remains the hero and changes with the real output power.
2. ✓ The English section labels felt imported from a design template. They were rewritten
   as concise German technical chapter markers.
3. ✓ Glow must communicate energy rather than decorate every surface. It is restricted to
   heater, live signal, sensor probe, state and safety points.
4. ✓ Different device classes need different choreography. Short desktop, tablet and phone
   now use independent stage heights, annotations and process rhythm.
5. – A photorealistic Three.js cup could impress in isolation, but would break the drawing
   language, budget and source parity. The authored SVG cutaway remains the product mark.

## Anti-AI / generic-dashboard audit

The audit explicitly searched for the common output patterns of generated dashboards.

- **Repeated rounded cards:** absent from the primary UI. Sections are separated by space,
  signal lines, typography and changes in visual density. Circles only describe actual
  process nodes or rotary interaction.
- **Glassmorphism everywhere:** rejected. Blur is restricted to the persistent header; the
  engineering drawer is an opaque technical layer.
- **Gradient decoration:** gradients have physical meaning (liquid temperature, heater
  plate, graph energy fill, thermal field). No rainbow or decorative gradient headline.
- **Pill overload:** absent. Range selection and state indicators are linear instruments,
  not pills.
- **Uniform card grids:** absent. Hero is a cutaway scene, control loop is a signal
  landscape, chart is a field, hardware is a radial system map, and events are a rail.
- **Tiny uppercase labels everywhere:** reduced in revision two. Chapter labels are German;
  uppercase monospace remains only for instrumentation, state and signal notation.
- **Glow everywhere:** rejected; see product-review point three.
- **Generic icon library:** none. The mark, motion, engineering, display and process symbols
  are authored CSS/SVG geometry.
- **Arbitrary rounded corners:** none on content regions. Circular geometry describes
  temperature control, connection status or system topology.
- **Stock chart component:** none. The chart is an authored Canvas renderer sharing the
  interface’s coordinate/grid language.

## Signature retained after the audit

1. **Thermal cutaway:** an instrumented Becher–plate–Peltier–fan section whose heat contours,
   plate energy and fan motion respond to live control values.
2. **Spatial feedback signal:** a large non-card PI chain with a geometrically separate
   return path and live values at the actual signal locations.
3. **Safety horizon:** seven interlocks share a single system line below the physical scene;
   it reads as one permission boundary rather than seven status cards.

These elements remain intentionally distinctive in both preview and the real embedded
single-file production build.
