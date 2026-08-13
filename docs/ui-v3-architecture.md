# V3 — Interactive Product Experience

## Secured baseline

- Branch: `feature`
- Restorable Observatory V2 commit: `e24098c97c19e75cfdfd0950487a0701772ff0b3`
- Local and `origin/feature` matched before V3 work began.
- The working tree was clean.

V3 is developed under `ui-v3/` until its generated preview and embedded asset pass the
existing dashboard, lwIP, controller and LED regressions. No firmware, API, network or
safety module is part of the visual rewrite.

## Product direction

V3 treats the physical heater as the interface. A controlled 3/4 product view occupies the
viewport; `w(t)`, comparator, PI, `u(t)`, Peltier, thermal plant, TMP36 and `y(t)` form a
spatial loop around the actual components. The lower horizon is the live timeline. Controls
are instruments embedded into this scene, not form widgets in cards.

Modes alter one scene rather than navigate between dashboard pages:

- **Overview:** essential process values, integrated loop and control.
- **Thermal:** heat field, energy transfer, sensor scan and live timeline receive emphasis.
- **Engineering:** exploded product, electronic nodes, power/sensor signal routes and PI
  decomposition.
- **Presentation:** full-screen product, loop and timeline with restrained supporting data.

## Technology experiments

Three executable comparisons live under `ui-v3/prototypes/`.

| Criterion | Three.js prototype | Custom WebGL prototype | SVG/Canvas continuation |
|---|---|---|---|
| Product depth | Excellent | Excellent for controlled camera | Good pseudo-3D |
| Lighting/materials | Rich PBR abstractions | Purpose-built analytical light | Authored gradients |
| Postprocessing | Easy but expensive | Single integrated shader pass | CSS/SVG filters |
| Preview size | Large external module | Small authored engine | Smallest |
| Pico flash | Poor unless heavily bundled | Good | Excellent |
| Draw-call control | Indirect | Exact | Not GPU-mesh based |
| Maintenance | Library/API upgrades | Small project-specific API | Familiar but complex SVG |
| Preview/production parity | Weak | Exact | Exact |

The Three.js experiment uses a real `Scene`, `PerspectiveCamera`, `WebGLRenderer`, physical
materials, multiple lights, fog, transparency and an exploded-state transition. It is not a
mock-up. The custom experiment creates the product from generated mesh primitives with a
small shader, controlled light/fog and exact draw-call accounting. The SVG/Canvas baseline is
represented by Observatory V2 and remains the no-WebGL fallback.

## Selected production direction

**Custom WebGL product renderer + DOM/SVG signal layer + Canvas timeline.**

Reasons:

1. The fixed product camera needs only a small mesh/material subset, not a general scene
   framework.
2. Preview and Pico can use the same renderer and authored shaders.
3. There is no CDN, npm runtime or second visual implementation.
4. Product state, exploded view and thermal energy can be expressed with exact shader
   uniforms and transforms.
5. SVG remains the clearest medium for control-theory notation and feedback paths.
6. Canvas remains appropriate for a data-heavy, continuously changing timeline.

Three.js remains a committed design/engineering benchmark, not production code. The latest
prototype reference during exploration was Three.js 0.185.1. No external library is shipped
to the Pico.

## Rendering architecture

```text
/api/status or preview simulator
                │
          normalized model
     ┌──────────┼───────────┐
 custom WebGL   SVG/DOM     Canvas
 product scene  control loop timeline
     └──────────┼───────────┘
           shared state/motion
```

- Product canvas: DPR capped, 60 FPS preview target, 30 FPS embedded/low-power target.
- WebGL fallback: authored SVG product scene, always present below the canvas.
- Control loop: SVG paths and semantic DOM values remain keyboard/screen-reader legible.
- Timeline: retained-mode data, one scheduled draw per status update.
- No response-sized buffers are added to the Pico.

## Control experiment

Three live controls are compared in `control-lab.html`:

1. Rotary thermal arc.
2. Vertical temperature rail.
3. Direct large plus/minus control.

The selected V3 control combines the directness of large ± targets with a spatial orbital
arc. Pure drag is not the only path: keyboard arrows and a numeric input remain available.
The vertical rail is excellent on mobile but weaker in the 3/4 desktop composition; it is
therefore used as a compact mobile adaptation, not the primary desktop instrument.

## Safety and data contracts

- START remains enabled only from the server-provided `start_allowed` state.
- The UI never predicts or grants safety.
- STOP remains available whenever the live API is reachable.
- Missing hardware renders as neutral/unavailable, never fabricated OK.
- ERROR removes thermal output visually and shows the server fault reason.
- OFFLINE freezes data, disables commands and preserves the product/history context.
- API endpoints and polling interval remain unchanged.
