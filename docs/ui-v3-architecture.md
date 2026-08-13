# V3/V4 product interface architecture

## Scope and baseline

The current interface continues on branch `feature` from stable baseline
`d29a6e55de549aaf733330dd6208564b62290b58`. It does not alter firmware state,
networking, API routes, authentication or safety decisions.

## Rendering stack

```text
/api/status or local preview simulator
                |
         normalized live state
      +---------+----------+
      |         |          |
 inline SVG   DOM/SVG    Canvas 2D
 product      control     timeline
      +---------+----------+
                |
       one generated HTML file
```

- `ui-v3/src/index.html` owns semantic sections, values and command controls.
- `ui-v3/src/product.svg` owns the readable product artwork. The build generator
  embeds it inline; it is not a second HTTP asset.
- `ui-v3/src/experience.css` owns the quiet four-section composition and the seven
  required responsive layouts.
- `ui-v3/src/experience.js` owns live rendering, charting and authenticated commands.
- `ui-v3/src/preview.js` owns demo data only and is excluded from production.
- `ui-v4/src/component-model.js` maps the nineteen real hardware concepts to API data.
- `ui-v4/src/digital-twin.js` provides direct SVG selection and preview-only add/remove.

There is no WebGL, Three.js, fake-3D canvas, X-Ray mode, scroll mode or remote asset.
The first frame is complete without JavaScript; JavaScript only adds live values,
component state, fan motion and interaction.

## Product model

The authored 2.5D projection uses one light direction and one material vocabulary:

- translucent, slightly conical drinks cup with a separate liquid surface;
- dark structural base, curved guides, upper ring and recessed lower support;
- thin ceramic Peltier stacks with dark intermediate layers;
- curved aluminium contact jaws at cup-wall height;
- two clipped TO-92 sensors with three routed leads each;
- tray-mounted project PCB with Pico W, USB shell, TLA2024, drivers, connectors,
  capacitors, traces, mounting holes and status LEDs;
- integrated OLED module and four physical front buttons;
- six-fin aluminium sink and screwed six-blade fan housing.

The heat state uses only restrained orange contact gradients. The fan rotates slowly only
when live data reports fan power. Reduced-motion clients receive static product state.

## Interaction and safety

Preview mode exposes six direct component controls: Sensor 1/2, Peltier 1/2, Display and
Fan. Missing components show a faint dashed installation silhouette and a small plus at the
real mounting position. The 44 px accessible hit area surrounds a 20–24 px visible marker.
A single click adds or removes the local illustration state.

Production never simulates hardware. It consumes only real API state. START and setpoint
remain protected by the Pico-issued token. STOP remains deliberately available without a
token whenever the live API is reachable. Firmware remains the sole safety authority.
