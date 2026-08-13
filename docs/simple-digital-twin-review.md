# Simple Digital Twin — final UX review

## Objective

The final UX pass replaces the persistent Engineering HUD with four progressively disclosed levels:
physical product, closed loop, live behavior and optional technical details. Read-only monitoring remains
open, while all HTTP write commands require a server-issued, short-lived authorization token.

## Review round 1 — reduction

- Removed the permanent IP, timestamp, modes, X-ray/signals controls, notation-heavy hero and six-step scroll HUD.
- Replaced them with AUFBAU, REGELKREIS and LIVE navigation plus an optional TECHNIK disclosure.
- Kept five product entry hotspots by default and moved the complete component list behind a clear action.
- Reduced component truth to GEPLANT, ANGESCHLOSSEN, LIVE and FEHLER where applicable.
- Preserved explicit `nicht direkt überwacht` and `nicht verfügbar` states instead of inventing discovery.
- Kept the WebGL product, its immediate SVG fallback and the first-frame data gate.

## Review round 2 — comprehension

- Replaced the circular/HUD control nodes with ordinary-language rectangular steps.
- Made technical notation `w(t)`, `e(t)`, `u(t)` and `y(t)` secondary.
- Used the recognizable cup-holder miniature as the plant rather than an abstract `G(s)` block.
- Added a straight feedback return and a separate one-column mobile composition.
- Reduced the live area to temperature, target, power, fan and state.
- Kept IST, SOLL and HEIZLEISTUNG visible by default; secondary channels require `Mehr anzeigen`.
- Replaced the orbit setpoint control with minus, value, plus and save.
- Made the locked state and server-side PIN action visually explicit.

## Review round 3 — responsive and truth

- Verified desktop and phone captures for AUFBAU, REGELKREIS and LIVE.
- Corrected deterministic section positioning for screenshot/review URLs.
- Prevented optional signal controls from overriding the HTML `hidden` state.
- Kept the preview component configurator hardware-free while physically hiding removed model groups.
- Kept the complete PCB, Pico W, display, four buttons, sensors, Peltier stack, sink and fan visible as one device.
- Added an automated three-section × seven-viewport collision and overflow audit.
- Preserved the large-response sender and raw-lwIP pbuf ownership contract.

## Authorization model

`POST /api/unlock` accepts the form field `pin`. The firmware compares the PIN, creates a random 64-bit
token using Pico entropy, and keeps up to four simultaneous RAM-only sessions. Tokens expire after five
minutes and are never persisted by the browser. START, STOP and SETPOINT reject a missing, invalid or
expired token before reaching their callbacks. A valid token grants only permission to request an action;
`start_allowed()` and `safety_can_start()` remain the independent and final heating authority.
