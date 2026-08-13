# V4 — Configurator / Digital Twin Experience

## Secured baseline

- Branch: `feature`
- Restorable V3 commit: `d678e687216df7c79e38c0d635ae125727dc76a6`
- Local and `origin/feature` matched before V4 work began.
- The working tree was clean.

V4 extends the proven V3 rendering and API pipeline. Firmware, safety, networking and HTTP
transport are deliberately outside the visual change. The embedded page still consumes only the
existing `/api/status`, `/api/start`, `/api/stop` and `/api/setpoint` contracts.

## Experience model

The page is one continuous product story with a sticky physical system rather than independent
dashboard sections:

1. **Physical system / Configure** — quiet premium product view, live temperature and optional
   component configuration in the local preview.
2. **Control loop** — the same physical product becomes the plant inside `w(t) → PI → u(t) → G(s)`.
3. **Live control** — setpoint and state control enter the scene without replacing the product.
4. **Live analysis** — the spatial timeline expands and exposes only meaningful available signals.
5. **Safety** — server-authoritative start permission and fault reason receive focus.
6. **Engineering** — X-ray/exploded hardware and electrical signal paths become visible.

The explicit top-level views answer separate questions:

- **Product:** What is this device?
- **Control:** How does the closed loop work?
- **Thermal:** Where does thermal energy flow?
- **Engineering:** Which hardware is present and what does the API actually know about it?

## Digital-twin truth model

Every component has three deliberately independent dimensions:

- `configured`: part of the planned PCB/system architecture;
- `connected`: preview-only simulated physical presence;
- `live`: derived only from a matching API field or clearly labelled inference.

`configured` never implies `detected`. Components without discovery telemetry (buttons, individual
H-bridges, a stopped fan) are labelled accordingly. Optional or disconnected hardware stays neutral
unless the firmware reports a related system fault. The UI never fabricates measurements.

The component catalogue in `ui-v4/src/component-model.js` records function, physical position,
signal, connection and pin/channel for the complete system. Boolean API fields are normalized as
`true`, `false` or `unknown`; missing fields therefore render as `NICHT VERFÜGBAR`, never as an
invented fault. Preview connection profiles mutate a local `Set` only; they cannot reach hardware.

## Source and build layering

```text
ui-v3/src/                 proven renderer, API client, control and chart foundation
ui-v4/src/
  component-model.js       hardware catalogue and truthful discovery mapping
  digital-twin.js          hotspots, focus, scroll story, guide and preview configuration
  digital-twin.css         V4 product-first composition and responsive behavior
          │
tools/build_ui.py
          ├── preview.html (simulator and configuration controls)
          └── include/web_assets.h (offline Pico production asset)
```

The production build remains a single offline document with zero external libraries and no runtime
asset requests. Three.js remains a prototype benchmark rather than a Pico dependency.
